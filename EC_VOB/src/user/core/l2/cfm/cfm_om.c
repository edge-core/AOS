/*-----------------------------------------------------------------------------
 * Module Name: cfm_om.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementation for the CFM object manager
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/5/2006 - macauley_cheng, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "l_hisam.h"
#include "cfm_om.h"
#include "cfm_backdoor.h"
#include "backdoor_mgr.h"
#include "l_cvrt.h"
#if (SYS_CPNT_CFM == TRUE)
/*-----------------
 * LOCAL CONSTANTS
 *-----------------*/

#define MEP_NUMBER_OF_KEYS 2
#define REMOTE_MEP_NUMBER_OF_KEYS 1
#define LTR_NUMBER_OF_KEYS 1
#define MIP_NUMBER_OF_KEYS 2

/* Field length definitions for HISAM table
 */
#define MD_LEN                              4
#define MA_LEN                              4
#define MEP_LEN                             4
#define LPORT_LEN                           4
#define SEQUENCE_LEN                        4
#define RECEIVE_ORDER_LEN                   4
#define MD_MA_MEP_KEY_LEN                   (MD_LEN + MA_LEN + MEP_LEN)
#define LPORT_MD_MA_KEY_LEN                 (MD_MA_MEP_KEY_LEN)
#define MD_MA_MEP_SEQUENCE_RECEIVE_KEY_LEN  (MD_MA_MEP_KEY_LEN + SEQUENCE_LEN + RECEIVE_ORDER_LEN)

#define MEP_NODE_LEN                        sizeof(CFM_OM_MEP_T)
#define REMOTE_MEP_NODE_LEN                 sizeof(CFM_OM_REMOTE_MEP_T)
#define LTR_NODE_LEN                        sizeof(CFM_OM_LTR_T)
#define MIP_NODE_LEN                        sizeof(CFM_OM_MIP_T)
/* key index for HISAM table
 */
#define LPORT_MD_MA_KIDX                    0 /* LPORT,MD,  MA  */
#define MD_MA_MEP_KIDX                      1 /* MD, MA, MEP ID */
#define MD_MA_LPORT_KIDX                    1 /* MD, MA, LPORT  */
#define REMOTE_MD_MA_MEP_KIDX               0
#define MD_MA_MEP_SEQ_REC_KIDX              0 /* MD, MA, MEP ID, SEQUENCE, RECEVIE ORDER */


/* constant definitions for HISAM
 */
#define INDEX_NBR                           10 /* table is divided to 100 blocks  */
#define HISAM_N1                            3  /* table balance threshold */
#define HISAM_N2                            13 /* table balance threshold */
#define HASH_DEPTH                          4

#define MEP_NODE_NBR                        SYS_ADPT_CFM_MAX_NBR_OF_MEP
#define MEP_HASH_NBR                        ((MEP_NODE_NBR+10) / 10)

#define MIP_NODE_NBR                        SYS_ADPT_CFM_MAX_NBR_OF_MIP
#define MIP_HASH_NBR                        ((MIP_NODE_NBR+10) / 10)


#define REMOTE_MEP_NODE_NBR                 SYS_ADPT_CFM_MAX_NBR_OF_REMOTE_MEP
#define REMOTE_MEP_HASH_NBR                 ((REMOTE_MEP_NODE_NBR+10) / 10)

#define LTR_NODE_NBR                        SYS_ADPT_CFM_MAX_NBR_OF_LTR
#define LTR_HASH_NBR                        ((LTR_NODE_NBR+10) / 10)


#define CFM_OM_LOCK()  {/*if(flag>0) printf("%s.%d, \t\t\tLOCK\n", __FUNCTION__, __LINE__); flag++;*/ SYSFUN_TakeSem(cfm_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);}
#define CFM_OM_UNLOCK() {/*flag--; if(flag>0 || flag<0)printf("%s.%d, \t\t\tUNLOCK\n", __FUNCTION__, __LINE__);*/SYSFUN_GiveSem(cfm_om_sem_id);/* flag=0*/}

typedef struct CFM_OM_ShmemData_S
{
    SYSFUN_DECLARE_CSC_ON_SHMEM

    L_HISAM_ShMem_Desc_T   mep_hisam_des_g;
    L_HISAM_ShMem_Desc_T   mip_hisam_des_g;
    L_HISAM_ShMem_Desc_T   remote_mep_hisam_des_g;
    L_HISAM_ShMem_Desc_T   ltr_hisam_des_g;

    CFM_OM_MD_T                 md_ag[SYS_ADPT_CFM_MAX_NBR_OF_MD];
    CFM_OM_MA_T                 ma_ag[SYS_ADPT_CFM_MAX_NBR_OF_MA];

    CFM_OM_GlobalConfig_T       global_cfg_g;
    CFM_OM_LTR_QUEUE_HEAD_T     ltr_queue_header_g;
    CFM_TYPE_CfmStatus_T        cfm_port_status_ag[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    CFM_OM_ErrorListElement_T   error_list_ag[SYS_ADPT_CFM_MAX_NBR_OF_ERROR];
    CFM_OM_ErrorListHead_T      error_list_head_g;
    CFM_OM_LoopBackListElement_T lbr_list_ag[CFM_TYPE_MAX_LBM_COUNT];
    CFM_OM_LoopBackListHead_T    loop_back_list_head_g;
    CFM_OM_DmmCtrlRec_T          dmm_ctr_rec_g;
    CFM_OM_LbmCtrlRec_T          lbm_ctr_rec_g;
    UI32_T                       link_trace_entries_g;
    UI32_T                       md_next_index_g;
    I16_T                        md_head_index;
    UI8_T                        buff[0];
}CFM_OM_ShmemData_T;

/*-------------------
 * Local VARIABLES
 *-------------------*/
const static L_HISAM_KeyDef_T mep_key_def_table[MEP_NUMBER_OF_KEYS] =
                {
                    /* ifinex, md, ma, */
                    {   3,                          /* field number */
                        {12, 0, 4, 0, 0, 0, 0, 0},  /* offset */
                        {4, 4, 4, 0, 0, 0, 0, 0}   /* len */
                    },
                    /*md, ma, mep_id*/
                    {   3,                         /* field number */
                        {0, 4, 8, 0, 0, 0, 0, 0},  /* offset */
                        { 4, 4, 4, 0, 0, 0, 0, 0}   /* len */
                    }
                };
const static L_HISAM_KeyDef_T mip_key_def_table[MIP_NUMBER_OF_KEYS] =
                {
                    /*lport, md, ma*/
                    {   3,                         /* field number */
                        {8, 0, 4, 0, 0, 0, 0, 0},  /* offset */
                        {4, 4, 4, 0, 0, 0, 0, 0}   /* len */
                    },
                    /*md, ma, lport*/
                    {   3,                         /* field number */
                        {0, 4, 8, 0, 0, 0, 0, 0},  /* offset */
                        {4, 4, 4, 0, 0, 0, 0, 0}   /* len */
                    }
                };

const static L_HISAM_KeyDef_T remote_mep_key_def_table[REMOTE_MEP_NUMBER_OF_KEYS] =
                {
                    /*md, ma, mep id*/
                    {   3,                         /* field number */
                        {0, 4, 8, 0, 0, 0, 0, 0},  /* offset */
                        {4, 4,4, 0, 0, 0, 0, 0}   /* len */
                    }

                };

const static L_HISAM_KeyDef_T ltr_key_def_table[LTR_NUMBER_OF_KEYS] =
                {
                    /*md, ma, mep id, seqnum, recevieOrder*/
                    {   5,                         /* field number */
                        {0, 4, 8, 12, 16, 0, 0, 0},  /* offset */
                        {4, 4, 4, 4, 4, 0, 0, 0}   /* len */
                    }
                };

/* md and ma index may be 1~0xffffffff,
 *   need to use L_HISAM_NOT_INTEGER to search the hisam table correctly.
 */
const static L_HISAM_KeyType_T mep_key_type_table[MEP_NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /*md, ma, mep id*/
                    {L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER},
                    /*md, ma, ifinex */
                    {L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER}
                };
const static L_HISAM_KeyType_T mip_key_type_table[MIP_NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /*lport, md, ma*/
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER},
                     /*md, ma, lport*/
                    {L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER}
                };
const static L_HISAM_KeyType_T remote_mep_key_type_table[REMOTE_MEP_NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /*md, ma, mep id*/
                    {L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER}
                };
const static L_HISAM_KeyType_T ltr_key_type_table[LTR_NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                   /*md, ma, mep id, seqnum, recevieOrder*/
                    {L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER}
                };


static CFM_OM_ShmemData_T   *sharmem_data_p;
static UI32_T               cfm_om_sem_id;
//static CFM_TIMER_HEAD_T     timer_head_g;   /*timer link list header*/

/* temp buffer for om/engine api
 */
CFM_OM_MEP_T mep_tmp2_g, mep_tmp3_g;
CFM_OM_LTR_T ltr_tmp_g;
static I16_T flag;

/*-------------------
 * Local SUBROUTINES
 *-------------------*/

static void CFM_OM_LocalCreateHisamTable();
static BOOL_T CFM_OM_LocalHisamSetRecord(
                            L_HISAM_ShMem_Desc_T *desc_p,
                            UI8_T *rec_p);
static BOOL_T CFM_OM_LocalHisamGetRecord(
                            L_HISAM_ShMem_Desc_T *desc_p,
                            UI32_T kidx,
                            UI8_T *key_ap,
                            UI8_T *rec_p);
static BOOL_T CFM_OM_LocalHisamGetNextRecord(
                            L_HISAM_ShMem_Desc_T *desc_p,
                            UI32_T kidx,
                            UI8_T *key_ap,
                            UI8_T *rec_p);
static BOOL_T CFM_OM_LocalHisamDeleteRecord(
                            L_HISAM_ShMem_Desc_T *desc_p,
                            UI8_T *key_ap);
static void CFM_OM_LocalSetMdMaMepKey(
                            UI8_T key_a[MD_MA_MEP_KEY_LEN],
                            UI32_T md,
                            UI32_T ma,
                            UI32_T mep);
static void CFM_OM_LocalSetLportMdMaKey(
                            UI8_T key_a[LPORT_MD_MA_KEY_LEN],
                            UI32_T md,
                            UI32_T ma,
                            UI32_T lport);
static void CFM_OM_LocalSetMdMaLportKey(
                            UI8_T key_a[LPORT_MD_MA_KEY_LEN],
                            UI32_T md,
                            UI32_T ma,
                            UI32_T lport);
static void CFM_OM_LocalSetMdMaSeqRecKey(
                            UI8_T key_a[MD_MA_MEP_SEQUENCE_RECEIVE_KEY_LEN],
                            UI32_T md,
                            UI32_T ma,
                            UI32_T mep,
                            UI32_T seq,
                            UI32_T order);
static BOOL_T CFM_OM_LocalSetMepRecord(
                            CFM_OM_MEP_T *mep_rec_p);
static BOOL_T CFM_OM_LocalDeleteMepRecord(
                            CFM_OM_MEP_T *mep_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalGetMepExactRecord(
                            CFM_OM_MEP_T *mep_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalGetNextMepRecord(
                            CFM_OM_MEP_T *mep_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalSetMipRecord(
                            CFM_OM_MIP_T *mip_rec_p);
static BOOL_T CFM_OM_LocalDeleteMipRecord(
                            CFM_OM_MIP_T *mip_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalGetMipExactRecord(
                            CFM_OM_MIP_T *mip_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalGetNextMipRecord(
                            CFM_OM_MIP_T *mip_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalSetRemoteMepRecord(
                            CFM_OM_REMOTE_MEP_T *mep_rec_p);
static BOOL_T CFM_OM_LocalDeleteRemoteMepRecord(
                            CFM_OM_REMOTE_MEP_T *mep_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalGetRemoteMepExactRecord(
                            CFM_OM_REMOTE_MEP_T *mep_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalGetNextRemoteMepRecord(
                            CFM_OM_REMOTE_MEP_T *mep_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalSetLtrRecord(
                            CFM_OM_LTR_T *ltr_rec_p);
static BOOL_T CFM_OM_LocalDeleteLtrRecord(
                            CFM_OM_LTR_T *ltr_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalGetLtrExactRecord(
                            CFM_OM_LTR_T *ltr_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalGetNextLtrRecord(
                            CFM_OM_LTR_T *ltr_rec_p,
                            UI16_T key_type);
static BOOL_T CFM_OM_LocalInitMd(
                            CFM_OM_MD_T *md_p,
                            UI8_T *name_ap,
                            UI32_T name_length,
                            UI32_T md_index,
                            CFM_TYPE_MdLevel_T level,
                            CFM_TYPE_MhfCreation_T create_way);
static BOOL_T CFM_OM_LocalInitMip(
                            CFM_OM_MIP_T *mip_p,
                            CFM_OM_MD_T *md_p,
                            CFM_OM_MA_T *ma_p,
                            UI32_T lport,
                            UI8_T mac_a[SYS_ADPT_MAC_ADDR_LEN]);

static UI32_T CFM_OM_LocalFindNextMdIndex(
                            UI32_T current_md_index);
static UI32_T CFM_OM_LocalFindNextMaIndex(
                            UI32_T md_index,
                            UI32_T current_ma_index);
static BOOL_T CFM_OM_CnvDataToOid(
    UI8_T   *in_data_p,
    UI32_T  in_datalen,
    UI32_T  *out_oid_p,
    UI32_T  *inout_oidlen_p);
static CFM_OM_ErrorListHead_T       *CFM_OM_LocalGetErrorListHeadPtr();
static CFM_OM_LoopBackListHead_T    *CFM_OM_LocalGetLoopBackListHeadPtr();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalCreateHisamTable
 * ------------------------------------------------------------------------
 * PURPOSE  : this function will create the om hisam table for MEP, remote MEP, LTR,MIP
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void CFM_OM_LocalCreateHisamTable()
{
    UI8_T *offset_now=0;
    offset_now+=sizeof(CFM_OM_ShmemData_T);

    sharmem_data_p->mep_hisam_des_g.hash_depth         = HASH_DEPTH;
    sharmem_data_p->mep_hisam_des_g.N1                 = HISAM_N1;
    sharmem_data_p->mep_hisam_des_g.N2                 = HISAM_N2;
    sharmem_data_p->mep_hisam_des_g.record_length      = MEP_NODE_LEN;
    sharmem_data_p->mep_hisam_des_g.total_hash_nbr     = MEP_HASH_NBR;
    sharmem_data_p->mep_hisam_des_g.total_index_nbr    = INDEX_NBR;
    sharmem_data_p->mep_hisam_des_g.total_record_nbr   = MEP_NODE_NBR;
    /*the wk_offset base addr is the hisam descriper, so descrese the offset from start to hisam descriper*/
    sharmem_data_p->mep_hisam_des_g.wk_offset = L_CVRT_GET_OFFSET(L_CVRT_GET_OFFSET(sharmem_data_p, &sharmem_data_p->mep_hisam_des_g), offset_now);

    offset_now+=L_HISAM_ShMem_GetWorkingBufferRequiredSize(&sharmem_data_p->mep_hisam_des_g, MEP_NUMBER_OF_KEYS);

     if(!L_HISAM_ShMem_CreateV2(&sharmem_data_p->mep_hisam_des_g, MEP_NUMBER_OF_KEYS, mep_key_def_table, mep_key_type_table))
    {
        printf(" MEP: Create HISAM Error !\n");
        while(1);
    }

    sharmem_data_p->mip_hisam_des_g.hash_depth         = HASH_DEPTH;
    sharmem_data_p->mip_hisam_des_g.N1                 = HISAM_N1;
    sharmem_data_p->mip_hisam_des_g.N2                 = HISAM_N2;
    sharmem_data_p->mip_hisam_des_g.record_length      = MIP_NODE_LEN;
    sharmem_data_p->mip_hisam_des_g.total_hash_nbr     = MIP_HASH_NBR;
    sharmem_data_p->mip_hisam_des_g.total_index_nbr    = INDEX_NBR;
    sharmem_data_p->mip_hisam_des_g.total_record_nbr   = MIP_NODE_NBR;
    sharmem_data_p->mip_hisam_des_g.wk_offset = L_CVRT_GET_OFFSET(L_CVRT_GET_OFFSET(sharmem_data_p, &sharmem_data_p->mip_hisam_des_g), offset_now);
    offset_now+=L_HISAM_ShMem_GetWorkingBufferRequiredSize(&sharmem_data_p->mip_hisam_des_g, MIP_NUMBER_OF_KEYS);

    if(!L_HISAM_ShMem_CreateV2(&sharmem_data_p->mip_hisam_des_g, MIP_NUMBER_OF_KEYS, mip_key_def_table, mip_key_type_table))
    {
        printf(" MIP: Create HISAM Error !\n");
        while(1);
    }

    sharmem_data_p->remote_mep_hisam_des_g.hash_depth         = HASH_DEPTH;
    sharmem_data_p->remote_mep_hisam_des_g.N1                 = HISAM_N1;
    sharmem_data_p->remote_mep_hisam_des_g.N2                 = HISAM_N2;
    sharmem_data_p->remote_mep_hisam_des_g.record_length      = REMOTE_MEP_NODE_LEN;
    sharmem_data_p->remote_mep_hisam_des_g.total_hash_nbr     = REMOTE_MEP_HASH_NBR;
    sharmem_data_p->remote_mep_hisam_des_g.total_index_nbr    = INDEX_NBR;
    sharmem_data_p->remote_mep_hisam_des_g.total_record_nbr   = REMOTE_MEP_NODE_NBR;
    sharmem_data_p->remote_mep_hisam_des_g.wk_offset = L_CVRT_GET_OFFSET(L_CVRT_GET_OFFSET(sharmem_data_p, &sharmem_data_p->remote_mep_hisam_des_g), offset_now);
    offset_now+=L_HISAM_ShMem_GetWorkingBufferRequiredSize(&sharmem_data_p->remote_mep_hisam_des_g, REMOTE_MEP_NUMBER_OF_KEYS);
    if(!L_HISAM_ShMem_CreateV2(&sharmem_data_p->remote_mep_hisam_des_g, REMOTE_MEP_NUMBER_OF_KEYS, remote_mep_key_def_table, remote_mep_key_type_table))
    {
        printf(" Remote MEP: Create HISAM Error !\n");
        while(1);
    }

    sharmem_data_p->ltr_hisam_des_g.hash_depth         = HASH_DEPTH;
    sharmem_data_p->ltr_hisam_des_g.N1                 = HISAM_N1;
    sharmem_data_p->ltr_hisam_des_g.N2                 = HISAM_N2;
    sharmem_data_p->ltr_hisam_des_g.record_length      = LTR_NODE_LEN;
    sharmem_data_p->ltr_hisam_des_g.total_hash_nbr     = LTR_HASH_NBR;
    sharmem_data_p->ltr_hisam_des_g.total_index_nbr    = INDEX_NBR;
    sharmem_data_p->ltr_hisam_des_g.total_record_nbr   = LTR_NODE_NBR;
    sharmem_data_p->ltr_hisam_des_g.wk_offset = L_CVRT_GET_OFFSET(L_CVRT_GET_OFFSET(sharmem_data_p, &sharmem_data_p->ltr_hisam_des_g), offset_now);
    offset_now+=L_HISAM_ShMem_GetWorkingBufferRequiredSize(&sharmem_data_p->ltr_hisam_des_g, LTR_NUMBER_OF_KEYS);
    if(!L_HISAM_ShMem_CreateV2(&sharmem_data_p->ltr_hisam_des_g, LTR_NUMBER_OF_KEYS, ltr_key_def_table, ltr_key_type_table))
    {
        printf(" LTR: Create HISAM Error !\n");
        while(1);
    }
}/*end of CFM_OM_CreateHisameTable*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalHisamSetRecord
 * ------------------------------------------------------------------------
 * PURPOSE  : the function set the record to the hisam table
 * INPUT    : *desc_p  -  the hisam descriptor
 *            *rec_p   - the record will be put into hisame table
 * OUTPUT   : None
 * RETUEN   : TRUE   - sucess
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalHisamSetRecord(
                                    L_HISAM_ShMem_Desc_T *desc_p,
                                    UI8_T *rec_p)
{
    UI32_T return_value;

    return_value = L_HISAM_ShMem_SetRecord (desc_p, rec_p, TRUE);

    if((L_HISAM_INSERT!=return_value) &&( L_HISAM_REPLACE!=return_value))
    {
        return FALSE;
    }
    return TRUE;;
}/*end of CFM_OM_LocalHisamSetRecord*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalHisamSetRecord
 * ------------------------------------------------------------------------
 * PURPOSE  : the function get the record from the hisam table
 * INPUT    : *desc_p-  the hisam descriptor
 *            *rec   - the record will be put into hisame table
 * OUTPUT   : None
 * RETUEN   : TRUE   - sucess
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalHisamGetRecord(
                                    L_HISAM_ShMem_Desc_T *desc_p,
                                    UI32_T kidx,
                                    UI8_T *key_ap,
                                    UI8_T *rec_p)
{
    BOOL_T return_value;

    return_value = L_HISAM_ShMem_GetRecord(desc_p, kidx, key_ap, rec_p);

    return return_value;
}/*end of CFM_OM_LocalHisamGetRecord*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalHisamGetNextRecord
 * ------------------------------------------------------------------------
 * PURPOSE  : the function get the next record from the hisam table
 * INPUT    : *desc  -  the hisam descriptor
 *            *rec   - the record will be put into hisame table
 * OUTPUT   : None
 * RETUEN   : TRUE   - sucess
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalHisamGetNextRecord(
                                        L_HISAM_ShMem_Desc_T *desc_p,
                                        UI32_T kidx,
                                        UI8_T *key_ap,
                                        UI8_T *rec_p)
{
    BOOL_T return_value;

    return_value = L_HISAM_ShMem_GetNextRecord(desc_p, kidx, key_ap, rec_p);

    return return_value;
}/*end of CFM_OM_LocalHisamGetNextRecord*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalHisamDeleteRecord
 * ------------------------------------------------------------------------
 * PURPOSE  : the function delet the record from the hisam table
 * INPUT    : *desc  -  the hisam descriptor
 *            *rec   - the record will be put into hisame table
 * OUTPUT   : None
 * RETUEN   : TRUE   - sucess
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalHisamDeleteRecord(
                                        L_HISAM_ShMem_Desc_T *desc_p,
                                        UI8_T *key_ap)
{
    BOOL_T return_value;

    return_value = L_HISAM_ShMem_DeleteRecord(desc_p, key_ap);

    return return_value;
}/*end of CFM_OM_LocalHisamDeleteRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalSetMdMaMepKey
 *------------------------------------------------------------------------------
 * Purpose: Generate the key to access HISAM entries
 * INPUT  : md_Index  - md index
 *          ma_index  - ma index
 *          mep_id    - mep id
 * OUTPUT : *key      - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
static void CFM_OM_LocalSetMdMaMepKey(
                                UI8_T key_a[MD_MA_MEP_KEY_LEN],
                                UI32_T md_Index,
                                UI32_T ma_index,
                                UI32_T mep_id)
{
    memcpy(key_a, &md_Index, MD_LEN);
    memcpy(key_a+MD_LEN, &ma_index, MA_LEN);
    memcpy(key_a+MD_LEN+MA_LEN, &mep_id, MEP_LEN);

}/*end of CFM_OM_LocalSetMdMaMepKey*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalSetLportMdMaKey
 *------------------------------------------------------------------------------
 * Purpose: Generate the key to access HISAM entries
 * INPUT  : md      - the md index
 *          ma      - the ma index
 *          LPORT   - the logical port
 * OUTPUT : *key    - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
static void CFM_OM_LocalSetLportMdMaKey(
                                    UI8_T key_a[LPORT_MD_MA_KEY_LEN],
                                    UI32_T md,
                                    UI32_T ma,
                                    UI32_T lport)
{
    memcpy(key_a, &lport, LPORT_LEN);
    memcpy(key_a+LPORT_LEN, &md, MD_LEN);
    memcpy(key_a+LPORT_LEN+MD_LEN, &ma, MA_LEN);
}/*end of CFM_OM_LocalSetLportMdMaKey*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalSetMdMaLportKey
 *------------------------------------------------------------------------------
 * Purpose: Generate the key to access HISAM entries
 * INPUT  : md      - the md index
 *          ma      - the ma index
 *          LPORT   - the logical port
 * OUTPUT : *key    - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
static void CFM_OM_LocalSetMdMaLportKey(
                                    UI8_T key_a[LPORT_MD_MA_KEY_LEN],
                                    UI32_T md,
                                    UI32_T ma,
                                    UI32_T lport)
{
    memcpy(key_a, &md, MD_LEN);
    memcpy(key_a+MD_LEN, &ma, MA_LEN);
    memcpy(key_a+MD_LEN+MA_LEN, &lport, LPORT_LEN);
}/*end of CFM_OM_LocalSetMdMaLportKey*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalSetMdMaSeqRecKey
 *------------------------------------------------------------------------------
 * Purpose: Generate the key to access HISAM entries
 * INPUT  : md     - the md index
 *          ma     - the ma index
 *          mep    - the mep id
 *          seq    - sequence
 *          order  - receive order
 * OUTPUT : UI8_T *key    - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
static void CFM_OM_LocalSetMdMaSeqRecKey(
                                    UI8_T key_a[MD_MA_MEP_SEQUENCE_RECEIVE_KEY_LEN],
                                    UI32_T md,
                                    UI32_T ma,
                                    UI32_T mep,
                                    UI32_T seq,
                                    UI32_T order)
{
    memcpy(key_a,&md, MD_LEN);
    memcpy(key_a+MD_LEN,&ma, MA_LEN);
    memcpy(key_a+MD_LEN+MA_LEN,&mep, MEP_LEN);
    memcpy(key_a+MD_LEN+MA_LEN+MEP_LEN,&seq, SEQUENCE_LEN);
    memcpy(key_a+MD_LEN+MA_LEN+MEP_LEN+SEQUENCE_LEN,&order, RECEIVE_ORDER_LEN);
}/*end of CFM_OM_LocalSetMdMaSeqRecKey*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalSetMepRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will set the record into hisam table wheather it is already exist or not.
 *          if exist, it will change record; if not exist, it will add this record.
 * INPUT  : *mep_rec_p  - mep record which want to set into mep hisame table
 * OUTPUT :
 * RETUEN : TRUE     - sucess
 *          FALSE    - fail
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalSetMepRecord(
                                CFM_OM_MEP_T *mep_rec_p)
{
    BOOL_T hisam_result=FALSE;

    hisam_result = CFM_OM_LocalHisamSetRecord(&sharmem_data_p->mep_hisam_des_g, (UI8_T *) mep_rec_p);

    return hisam_result;
}/*end of CFM_OM_LocalSetMepRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalDeleteMepRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will delete the record from mep hisam table
 * INPUT  : *mep_rec_p - which mep record wil be delete
 *          key_type - which key type
 * OUTPUT :
 * RETUEN : TRUE     - sucess
 *          FALSE    - fail
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalDeleteMepRecord(
                                    CFM_OM_MEP_T *mep_rec_p,
                                    UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_LPORT_MD_MA_KEY)
    {
        CFM_OM_LocalSetLportMdMaKey( key, mep_rec_p->md_index, mep_rec_p->ma_index, mep_rec_p->lport);
        hisam_result = CFM_OM_LocalHisamDeleteRecord(&sharmem_data_p->mep_hisam_des_g, key);
    }
    else
    {
        printf("\n CFM_OM_LocalDeleteMepRecord the wrong key type, it should use the primary key");
        return FALSE;
    }

    return hisam_result;
}/*end of CFM_OM_LocalDeleteMepRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalGetMepExactRecord
 *------------------------------------------------------------------------------
 * Purpose : This function will get the record accroding to the key
 * INPUT   : *mep_rec_p - put the key in this variable
 * OUTPUT  : mep_rec - the output recrod get from the hisame table
 * RETURN  : TRUE    - get the record success
 *           FAIL    - get the recrod fail, it may not exist
 * NOTES   :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalGetMepExactRecord(
                                        CFM_OM_MEP_T *mep_rec_p,
                                        UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_MD_MA_MEP_KEY)
    {
        CFM_OM_LocalSetMdMaMepKey( key, mep_rec_p->md_index, mep_rec_p->ma_index, mep_rec_p->identifier);
        hisam_result = CFM_OM_LocalHisamGetRecord(&sharmem_data_p->mep_hisam_des_g, MD_MA_MEP_KIDX, key, (UI8_T *) mep_rec_p);
    }
    else if(key_type == CFM_OM_LPORT_MD_MA_KEY)
    {
        CFM_OM_LocalSetLportMdMaKey( key, mep_rec_p->md_index, mep_rec_p->ma_index, mep_rec_p->lport);
        hisam_result = CFM_OM_LocalHisamGetRecord(&sharmem_data_p->mep_hisam_des_g, LPORT_MD_MA_KIDX, key, (UI8_T *) mep_rec_p);
    }
    else
    {
        return FALSE;
    }

    return hisam_result;
}/*end of CFM_OM_LocalGetMepExactRecord*/


/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalGetNextMepRecord
 *------------------------------------------------------------------------------
 * Purpose:This function will get the record accroding to the input record
 * INPUT   : *mep_rec_p  - put the key in this variable
 *           key_type - use which key type
 * OUTPUT  : mep_rec  - the output recrod get from the hisame table
 * RETURN  : TRUE - get the record success
 *           FAIL - get the recrod fail, it may not exist
 * NOTES   :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalGetNextMepRecord(
                                CFM_OM_MEP_T *mep_rec_p,
                                UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_MD_MA_MEP_KEY)
    {
        CFM_OM_LocalSetMdMaMepKey( key, mep_rec_p->md_index, mep_rec_p->ma_index, mep_rec_p->identifier);
        hisam_result = CFM_OM_LocalHisamGetNextRecord(&sharmem_data_p->mep_hisam_des_g, MD_MA_MEP_KIDX, key, (UI8_T *) mep_rec_p);
    }
    else if(key_type == CFM_OM_LPORT_MD_MA_KEY)
    {
        CFM_OM_LocalSetLportMdMaKey( key, mep_rec_p->md_index, mep_rec_p->ma_index, mep_rec_p->lport);
        hisam_result = CFM_OM_LocalHisamGetNextRecord(&sharmem_data_p->mep_hisam_des_g, LPORT_MD_MA_KIDX, key, (UI8_T *) mep_rec_p);
    }
    else
    {
        printf("\n CFM_OM_LocalGetNextMepRecord the wrong key type");
        return FALSE;
    }

    return hisam_result;
}/*end of CFM_OM_LocalGetNextMepRecord*/


/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalSetMipRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will set the record into hisam table wheather it is already exist or not.
 *          if exist, it will change record; if not exist, it will add this record.
 * INPUT  : *mip_rec_p - mip record which want to set into mip hisame table
 * OUTPUT :
 * RETURN : TRUE     - set sucess
 *          FAIL     - set fail
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalSetMipRecord(
                                CFM_OM_MIP_T *mip_rec_p)
{
    BOOL_T hisam_result=FALSE;

    hisam_result = CFM_OM_LocalHisamSetRecord(&sharmem_data_p->mip_hisam_des_g,(UI8_T *) mip_rec_p);

    return hisam_result;
}/*end of CFM_OM_LocalSetMipRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalDeleteMipRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will delete the record from mip hisam table
 * INPUT  : *mip_rec_p  - which ltr wil be delete
 *          key_type    - which key type
 * OUTPUT :
 * RETURN : TRUE        - delete success
 *          FAILE       - delete faile
 * NOTES  :   delet only can use the primary key
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalDeleteMipRecord(
                                    CFM_OM_MIP_T *mip_rec_p,
                                    UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(CFM_OM_LPORT_MD_MA_KEY == key_type)
    {
        CFM_OM_LocalSetLportMdMaKey( key, mip_rec_p->md_index, mip_rec_p->ma_index, mip_rec_p->lport);
        hisam_result = CFM_OM_LocalHisamDeleteRecord(&sharmem_data_p->mip_hisam_des_g, key);
    }
    else
    {
        printf("\n CFM_OM_LocalDeleteMipRecord the wrong key type, it should use the primary key");
        return FALSE;
    }

    return hisam_result;
}/*end of CFM_OM_LocalDeleteMipRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalGetMipExactRecord
 *------------------------------------------------------------------------------
 * Purpose : This function will get the record accroding to the key
 * INPUT   : *mip_rec_p - put the key in this variable
 * OUTPUT  : *mip_rec_p - the output recrod get from the hisame table
 * RETURN  : TRUE     - get the record success
 *           FAIL     - get the recrod fail, it may not exist
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalGetMipExactRecord(
                                    CFM_OM_MIP_T *mip_rec_p,
                                    UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(CFM_OM_LPORT_MD_MA_KEY == key_type)
    {
        CFM_OM_LocalSetLportMdMaKey( key, mip_rec_p->md_index, mip_rec_p->ma_index, mip_rec_p->lport);
        hisam_result = CFM_OM_LocalHisamGetRecord(&sharmem_data_p->mip_hisam_des_g, LPORT_MD_MA_KIDX, key, (UI8_T *) mip_rec_p);
    }
    else if(CFM_OM_MD_MA_LPORT_KEY == key_type)
    {
        CFM_OM_LocalSetMdMaLportKey( key, mip_rec_p->md_index, mip_rec_p->ma_index, mip_rec_p->lport);
        hisam_result = CFM_OM_LocalHisamGetRecord(&sharmem_data_p->mip_hisam_des_g, MD_MA_LPORT_KIDX, key, (UI8_T *) mip_rec_p);
    }
    else
    {
        return FALSE;
    }

    return hisam_result;
}/*end of CFM_OM_LocalGetMipExactRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalGetNextMipRecord
 *------------------------------------------------------------------------------
 * Purpose :This function will get the record accroding to the input record
 * INPUT   : *mep_rec - put the key in this variable
 *           key_type - use which key type
 * OUTPUT  : mep_rec  - the output recrod get from the hisame table
 * RETURN  : TRUE     - get the record success
 *           FAIL     - get the recrod fail, it may not exist
 * NOTES   :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalGetNextMipRecord(
                                    CFM_OM_MIP_T *mip_rec_p,
                                    UI16_T key_type)
{
    UI8_T key[LPORT_MD_MA_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if( CFM_OM_LPORT_MD_MA_KEY == key_type)
    {
        CFM_OM_LocalSetLportMdMaKey( key, mip_rec_p->md_index, mip_rec_p->ma_index, mip_rec_p->lport);
        hisam_result = CFM_OM_LocalHisamGetNextRecord(&sharmem_data_p->mip_hisam_des_g, LPORT_MD_MA_KIDX, key, (UI8_T *) mip_rec_p);
    }
    else if(CFM_OM_MD_MA_LPORT_KEY == key_type)
    {
        CFM_OM_LocalSetMdMaLportKey( key, mip_rec_p->md_index, mip_rec_p->ma_index, mip_rec_p->lport);
        hisam_result = CFM_OM_LocalHisamGetNextRecord(&sharmem_data_p->mip_hisam_des_g, MD_MA_LPORT_KIDX, key, (UI8_T *) mip_rec_p);
    }

    else
    {
        return FALSE;
    }

    return hisam_result;
}/*end of CFM_OM_LocalGetNextMipRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalSetRemoteMepRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will set the record into hisam table wheather it is already exist or not.
 *          if exist, it will change record; if not exist, it will add this record.
 * INPUT  : *mep_rec_p - mep record which want to set into remote mep hisame table
 * OUTPUT :
 * RETURN : TRUE    - set sucess
 *          FAIL    - set fail
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalSetRemoteMepRecord(
                                CFM_OM_REMOTE_MEP_T *mep_rec_p)
{
    BOOL_T hisam_result=FALSE;

    hisam_result = CFM_OM_LocalHisamSetRecord(&sharmem_data_p->remote_mep_hisam_des_g,(UI8_T *) mep_rec_p);

    return hisam_result;
}/*end of CFM_OM_LocalSetRemoteMepRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalDeleteRemoteMepRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will delete the record from remote mep hisam table
 * INPUT  : *mep_rec_p - which mep wil be delete
 *          key-type   - which key type
 * OUTPUT :
 * RETURN : TRUE      - delete success
 *          FAILE     - delete faile
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalDeleteRemoteMepRecord(
                                        CFM_OM_REMOTE_MEP_T *mep_rec_p,
                                        UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_MD_MA_MEP_KEY)
    {
        CFM_OM_LocalSetMdMaMepKey( key, mep_rec_p->md_index, mep_rec_p->ma_index, mep_rec_p->identifier);
        hisam_result = CFM_OM_LocalHisamDeleteRecord(&sharmem_data_p->remote_mep_hisam_des_g, key);
    }
    else
    {
        printf("\n CFM_OM_LocalDeleteRemoteMepRecord the wrong key type");
        return FALSE;
    }

    return hisam_result;
}/*end of CFM_OM_LocalDeleteRemoteMepRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalGetRemoteMepExactRecord
 *------------------------------------------------------------------------------
 * Purpose:This function will get the record accroding to the key
 * INPUT  :*mep_rec_p - put the key in this variable
 * OUTPUT :*mep_rec_p - the output recrod get from the hisame table
 * RETURN :TRUE     - get the record success
 *         FAIL     - get the recrod fail, it may not exist
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalGetRemoteMepExactRecord(
                                            CFM_OM_REMOTE_MEP_T *mep_rec_p,
                                            UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_MD_MA_MEP_KEY)
    {
        CFM_OM_LocalSetMdMaMepKey( key, mep_rec_p->md_index, mep_rec_p->ma_index, mep_rec_p->identifier);
        hisam_result = CFM_OM_LocalHisamGetRecord(&sharmem_data_p->remote_mep_hisam_des_g, REMOTE_MD_MA_MEP_KIDX, key, (UI8_T *) mep_rec_p);
    }
    else
    {
        printf("\n key type is wrong");
        return FALSE;
    }
    return hisam_result;
}/*end of CFM_OM_LocalGetRemoteMepExactRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalGetNextRemoteMepRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will get the record accroding to the input record
 * INPUT  : *mep_rec_p- put the key in this variable
 *          key_type  - use which key type
 * OUTPUT : *mep_rec_p- the output recrod get from the hisame table
 * RETURN : TRUE - get the record success
 *          FAIL - get the recrod fail, it may not exist
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalGetNextRemoteMepRecord(
                                            CFM_OM_REMOTE_MEP_T *mep_rec_p,
                                            UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_MD_MA_MEP_KEY)
    {
        CFM_OM_LocalSetMdMaMepKey( key, mep_rec_p->md_index, mep_rec_p->ma_index, mep_rec_p->identifier);
        hisam_result = CFM_OM_LocalHisamGetNextRecord(&sharmem_data_p->remote_mep_hisam_des_g, REMOTE_MD_MA_MEP_KIDX, key, (UI8_T *) mep_rec_p);
    }
    else
    {
        printf("\n CFM_OM_LocalGetNextRemoteMepRecord the wrong key type");
        return FALSE;
    }
    return hisam_result;
}/*end of CFM_OM_LocalGetNextRemoteMepRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalSetLtrRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will set the record into hisam table wheather it is already exist or not.
 *          if exist, it will change record; if not exist, it will add this record.
 * INPUT  : *ltr_rec_ - mep record which want to set into ltr hisame table
 * OUTPUT :
 * RETURN : TRUE - set sucess
 *          FAIL - set fail
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalSetLtrRecord(
                                  CFM_OM_LTR_T *ltr_rec_p)
{
    BOOL_T hisam_result=FALSE;

    hisam_result = CFM_OM_LocalHisamSetRecord(&sharmem_data_p->ltr_hisam_des_g, (UI8_T *) ltr_rec_p);
    return hisam_result;
}/*end of CFM_OM_LocalSetLtrRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalDeleteLtrRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will delete the record from ltr hisam table
 * INPUT  : *ltr_rec_p  - which ltr wil be delete
 *          key-type    - which key type
 * OUTPUT :
 * RETURN : TRUE- delete success
 *          FAILE - delete faile
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalDeleteLtrRecord(
                                    CFM_OM_LTR_T *ltr_rec_p,
                                    UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_SEQUENCE_RECEIVE_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_MD_MA_MEP_SEQ_REC_KEY)
    {
        CFM_OM_LocalSetMdMaSeqRecKey( key, ltr_rec_p->md_index, ltr_rec_p->ma_index, ltr_rec_p->rcvd_mep_id, ltr_rec_p->seq_number, ltr_rec_p->receive_order);
        hisam_result = CFM_OM_LocalHisamDeleteRecord(&sharmem_data_p->ltr_hisam_des_g, key);
    }
    else
    {
        printf("\n CFM_OM_LocalGetLtrExactRecord the wrong key type");
        return FALSE;
    }
    return hisam_result;
}/*end of CFM_OM_LocalDeleteLtrRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalGetLtrExactRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will get the record accroding to the key
 * INPUT  : *ltr_rec_p- put the key in this variable
 * OUTPUT : *ltr_rec_p- the output recrod get from the hisame table
 * RETURN : TRUE - get the record success
 *          FAIL - get the recrod fail, it may not exist
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalGetLtrExactRecord(
                                    CFM_OM_LTR_T *ltr_rec_p,
                                    UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_SEQUENCE_RECEIVE_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_MD_MA_MEP_SEQ_REC_KEY)
    {
        CFM_OM_LocalSetMdMaSeqRecKey( key, ltr_rec_p->md_index, ltr_rec_p->ma_index, ltr_rec_p->rcvd_mep_id, ltr_rec_p->seq_number, ltr_rec_p->receive_order);
        hisam_result = CFM_OM_LocalHisamGetRecord(&sharmem_data_p->ltr_hisam_des_g, MD_MA_MEP_SEQ_REC_KIDX, key, (UI8_T *) ltr_rec_p);
    }
    else
    {
        printf("\n CFM_OM_LocalGetLtrExactRecord the wrong key type");
        return FALSE;
    }
    return hisam_result;
}/*end of CFM_OM_LocalGetLtrExactRecord*/

/*------------------------------------------------------------------------------
 * Function : CFM_OM_LocalGetNextLtrRecord
 *------------------------------------------------------------------------------
 * Purpose: This function will get the next record accroding to the input record
 * INPUT  : *ltr_rec_p  - put the key in this variable
 *          key_type    - use which key type
 * OUTPUT : *ltr_rec_p  - the output recrod get from the hisame table
 * RETURN : TRUE - get the record success
 *          FAIL - get the recrod fail, it may not exist
 * NOTES  :
 *------------------------------------------------------------------------------*/
static BOOL_T CFM_OM_LocalGetNextLtrRecord(
                                CFM_OM_LTR_T *ltr_rec_p,
                                UI16_T key_type)
{
    UI8_T key[MD_MA_MEP_SEQUENCE_RECEIVE_KEY_LEN];
    BOOL_T hisam_result=FALSE;

    if(key_type == CFM_OM_MD_MA_MEP_SEQ_REC_KEY)
    {
        CFM_OM_LocalSetMdMaSeqRecKey( key, ltr_rec_p->md_index, ltr_rec_p->ma_index, ltr_rec_p->rcvd_mep_id, ltr_rec_p->seq_number, ltr_rec_p->receive_order);
        hisam_result = CFM_OM_LocalHisamGetNextRecord(&sharmem_data_p->ltr_hisam_des_g, MD_MA_MEP_SEQ_REC_KIDX, key, (UI8_T *) ltr_rec_p);
    }
    else
    {
        return FALSE;
    }
    return hisam_result;
}/*end of CFM_OM_LocalGetNextLtrRecord*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetNextLoopBackElement
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the next loop back
 * INPUT    : seq_num            - the next sequal number
 *            loop_back_pp.index - the loop back index
 * OUTPUT   : loop_back_pp       - the next loop back pointer
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : if *error_pp is NULL, then return the fisrt element in list
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetNextLoopBackElement(
                            UI32_T   seq_num,
                            CFM_OM_LoopBackListElement_T **loop_back_pp)
{
    CFM_OM_LoopBackListHead_T *loop_back_list_head = CFM_OM_LocalGetLoopBackListHeadPtr();
    I16_T cur_lb_ar_idx = -1;

    if(-1 == loop_back_list_head->first_ar_idx)
    {
        return FALSE;
    }
    cur_lb_ar_idx=loop_back_list_head->first_ar_idx;

    while(cur_lb_ar_idx >= 0
        && TRUE == sharmem_data_p->lbr_list_ag[cur_lb_ar_idx].used)
    {
        if(sharmem_data_p->lbr_list_ag[cur_lb_ar_idx].lbm_seq_number> seq_num)
        {
            *loop_back_pp=&sharmem_data_p->lbr_list_ag[cur_lb_ar_idx];
            return TRUE;
        }
        cur_lb_ar_idx=sharmem_data_p->lbr_list_ag[cur_lb_ar_idx].next_ar_idx;
    }
    return FALSE;
}/*end of CFM_OM_LocalGetNextLoopBackElement*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalInitMd
 * ------------------------------------------------------------------------
 * PURPOSE  : the function will set the md to defalut value
 * INPUT    : md_p     - the mep which want to set to default
 *            md_index - the md index
 *            level    - the md level
 * OUTPUT   :
 * RETUEN   : TRUE  - success
 *            FALSE -fail.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalInitMd(
                            CFM_OM_MD_T *md_p,
                            UI8_T *name_ap,
                            UI32_T name_length,
                            UI32_T md_index,
                            CFM_TYPE_MdLevel_T level,
                            CFM_TYPE_MhfCreation_T create_way)
{
    memset(md_p, 0,sizeof(CFM_OM_MD_T));

    md_p->next_ar_idx=-1;
    md_p->ma_start_ar_idx=-1;

    memcpy(md_p->name_a, name_ap, name_length);
    md_p->name_length=name_length;
    md_p->index=md_index;
    md_p->level=level;
    md_p->format=CFM_TYPE_MD_NAME_CHAR_STRING;
    md_p->mhf_creation=create_way;

    md_p->mep_achive_holdtime=SYS_DFLT_CFM_ARCHIVE_HOLD_TIME*60;
    md_p->fng_alarm_time=SYS_DFLT_CFM_FNG_LOWEST_ALARM_TIME;
    md_p->fng_reset_time=SYS_DFLT_CFM_FNG_LOWEST_RESET_TIME;
    md_p->low_pri_def=SYS_DFLT_CFM_FNG_LOWEST_ALARM_PRI;
    md_p->mhf_id_permission=CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER;

    md_p->next_available_ma_index=1;
    md_p->used=TRUE;
    md_p->ar_idx = -1;
    return TRUE;
}/*end of CFM_OM_LocalInitMd*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalInitMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function initial ma
 * INPUT    : ma_p        - ma pointer
 *            ma_index    - the ma index
 *            *name_ap    - the ma array pointer
 *            name_length - the ma name length
 *            *md_p       - the md pointer
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalInitMa(
                    CFM_OM_MA_T *ma_p,
                    UI32_T ma_index,
                    UI8_T *name_ap,
                    UI32_T name_length,
                    CFM_OM_MD_T *md_p,
                    CFM_TYPE_MhfCreation_T create_way)
{
    memset(ma_p, 0, sizeof(CFM_OM_MA_T));

    ma_p->next_ar_idx = -1;
    ma_p->ar_idx = -1;

    ma_p->index = ma_index;
    ma_p->format=CFM_TYPE_MA_NAME_CHAR_STRING;

    ma_p->name_length=name_length;

    if(0!=name_length)
    {
        memcpy(ma_p->name_a, name_ap, name_length);
    }

    ma_p->md_ar_idx=md_p->ar_idx;

    ma_p->ccm_interval= SYS_DFLT_CFM_CCM_INTERVAL;
    ma_p->mhf_creation=SYS_DFLT_CFM_MIP_CREATE;
    ma_p->cross_check_status = SYS_DFLT_CFM_CROSSCHECK_STATUS;
    ma_p->ccm_status=SYS_DFLT_CFM_CCM_STATUS;
    ma_p->ais_period = SYS_DFLT_CFM_AIS_PERIOD;
    ma_p->ais_send_level = SYS_DFLT_CFM_AIS_LEVEL;
    ma_p->ais_status = SYS_DFLT_CFM_AIS_STATUS;
    ma_p->ais_supress_status = SYS_DFLT_CFM_AIS_SUPRESS_STATUS;
    ma_p->mhf_id_permission=SYS_DFLT_CFM_MHF_ID_PERMISSION;
    ma_p->mhf_creation = create_way;
    ma_p->ais_rcvd_timer_idx=-1;
    ma_p->used = TRUE;
    return TRUE;
}/*End of CFM_OM_LocalInitMa*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalInitMep
 * ------------------------------------------------------------------------
 * PURPOSE  : this function will set default value to mep
 * INPUT    : mep   - the mep which want to set to default
 * OUTPUT   :
 * RETUEN   : TRUE  - success
 *            FALSE -fail.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalInitMep(
                        CFM_OM_MD_T *md_p,
                        CFM_OM_MA_T *ma_p,
                        CFM_OM_MEP_T *mep_p,
                        UI32_T mep_id,
                        UI32_T lport,
                        CFM_TYPE_MP_Direction_T direction)
{
    memset(mep_p, 0,sizeof(CFM_OM_MEP_T));

    /*below just assing the default not 0 , null or FALSE, beause their value all are 0*/
    mep_p->md_ar_idx=md_p->ar_idx;
    mep_p->ma_ar_idx=ma_p->ar_idx;
    mep_p->ma_p=ma_p;
    mep_p->md_p=md_p;

    mep_p->identifier = mep_id;
    mep_p->lport=lport;
    mep_p->primary_vid = ma_p->primary_vid;
    mep_p->direction=direction;
    mep_p->active=SYS_DFLT_CFM_MEP_ACTIVE;  /*we have no cli command to set this inaction*/
    mep_p->cci_status=ma_p->ccm_status;
    mep_p->enable_rmep_defect=TRUE;/*we have no cli command to set this disable*/

    mep_p->ccm_ltm_priority=CFM_TYPE_DFT_CCM_PRIORITY;

    mep_p->fng_machine_state=CFM_TYPE_FNG_STATE_RESET;
    mep_p->highest_pri_defect=CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE;
    mep_p->last_notify_defect=0xffff;
    mep_p->lowest_priority_defect= md_p->low_pri_def;
    mep_p->fng_alarm_time=md_p->fng_alarm_time;
    mep_p->fng_reset_time=md_p->fng_reset_time;

    /*lbm*/
    mep_p->next_lbm_trans_id=CFM_TYPE_DFT_NEXT_TRANSAS_ID;
    mep_p->tramsmit_lbm_status=TRUE;
    memset(mep_p->transmit_lbm_dest_mac_address_a, 0, SYS_ADPT_MAC_ADDR_LEN);
    mep_p->transmit_lbm_messages=CFM_TYPE_DFT_TRANSMIT_LBM;
    mep_p->transmit_lbm_vlan_priority=CFM_TYPE_DFT_CCM_PRIORITY;    /*12.14.7.3.2:d*/
    mep_p->transmit_lbm_vlan_drop_enable=TRUE;
    mep_p->transmit_lbm_result_oK=TRUE;
    mep_p->transmit_lbm_data_tlv_length=CFM_TYPE_DFT_DATA_TLV_LEN;
    mep_p->transmit_lbm_seq_number=CFM_TYPE_DFT_SEQUENCE_NUM;

    /*ltm*/
    mep_p->ltm_next_seq_number=CFM_TYPE_DFT_NEXT_TRANSAS_ID;
    mep_p->transmit_ltm_status=TRUE;
    mep_p->transmit_ltm_result=TRUE;
    mep_p->transmit_ltm_seq_number=CFM_TYPE_DFT_SEQUENCE_NUM;
    mep_p->transmit_ltm_target_is_mep_id=FALSE;
    mep_p->transmit_ltm_ttl=SYS_DFLT_CFM_LTM_TTL;

    mep_p->cci_while_timer_idx=-1;
    mep_p->error_ccm_while_timer_idx=-1;
    mep_p->xcon_ccm_while_timer_idx=-1;
    mep_p->fng_while_timer_idx=-1;
    mep_p->ais_send_timer_idx=-1;

    return TRUE;
}/*end of CFM_OM_LocalInitMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalInitMip
 * ------------------------------------------------------------------------
 * PURPOSE  : the function will fill the value of mip structure
 * INPUT    : *mip_p - the mip which will fill the structure value
 *            *md_p  - the md pointer
 *            *ma_p  - the ma pointer
 *            lport  - the logical port
 *            mac_a- the mac address
 * OUTPUT   :
 * RETUEN   : TRUE  - success
 *            FALSE -fail.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalInitMip(
                            CFM_OM_MIP_T *mip_p,
                            CFM_OM_MD_T *md_p,
                            CFM_OM_MA_T *ma_p,
                            UI32_T lport,
                            UI8_T mac_a[SYS_ADPT_MAC_ADDR_LEN])
{
    mip_p->md_ar_idx=md_p->ar_idx;
    mip_p->ma_ar_idx=ma_p->ar_idx;
    mip_p->ma_p=ma_p;
    mip_p->md_p=md_p;
    mip_p->lport=lport;
    memcpy(mip_p->mac_address_a, mac_a, SYS_ADPT_MAC_ADDR_LEN);
    mip_p->md_index=md_p->index;
    mip_p->ma_index=ma_p->index;

    return TRUE;
}/*end of CFM_OM_LocalInitMip*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalInitRemoteMep
 *-------------------------------------------------------------------------
 * PURPOSE  : This function initial the remote mep
 * INPUT    : *r_mep_p   - the remote mep pointer
 *            r_mep_id          - the remote mep id
 *            *md_p           - the md pointer
 *            *ma_p           - the ma pointer
 * OUTPUT   : *remote_mep_p   - the remote mep pointer
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_OM_LocalInitRemoteMep(
                        CFM_OM_REMOTE_MEP_T *r_mep_p,
                        UI32_T r_mep_id,
                        CFM_OM_MD_T *md_p,
                        CFM_OM_MA_T *ma_p)
{
    memset(r_mep_p, 0, sizeof(CFM_OM_REMOTE_MEP_T));

    /*below just assing the value, which default not 0
     */
    r_mep_p->md_index=md_p->index;
    r_mep_p->ma_index=ma_p->index;
    r_mep_p->identifier=r_mep_id;

    r_mep_p->ma_ar_idx=ma_p->ar_idx;
    r_mep_p->md_ar_idx=md_p->ar_idx;

    /*assign md info*/
    r_mep_p->md_p=md_p;
    /*assign ma info*/
    r_mep_p->ma_p=ma_p;
    ma_p->remote_mep_down_counter ++;

    /*assign mep id*/
    r_mep_p->rcvd_mep_id=0;
    r_mep_p->rcvd_mep_direction=CFM_TYPE_MP_DIRECTION_UP_DOWN;

    /*assign next expect sequence*/
    r_mep_p->next_sequence=1;

    /*set remote mep state machine state*/
    r_mep_p->machine_state = CFM_TYPE_REMOTE_MEP_STATE_START;

    r_mep_p->rdi=FALSE;
    r_mep_p->ccm_defect=FALSE;

    r_mep_p->interface_status=CFM_TYPE_INTERFACE_STATUS_NO_INTERFACE_STATUS_TLV;
    r_mep_p->port_status=CFM_TYPE_PORT_STATUS_NO_PORT_STATE_TLV;

    r_mep_p->rcvd_lport = 0;

    r_mep_p->mac_addr_a[0]=0xff;
    r_mep_p->mac_addr_a[1]=0xff;
    r_mep_p->mac_addr_a[2]=0xff;
    r_mep_p->mac_addr_a[3]=0xff;
    r_mep_p->mac_addr_a[4]=0xff;
    r_mep_p->mac_addr_a[5]=0xff;
    r_mep_p->rmep_while_timer_idx=-1;
    r_mep_p->archive_hold_timer_idx=-1;
    return;
}/* End of CFM_OM_LocalInitRemoteMep*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_LocalGetFreeMdAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function find the element in sharmem_data_p->md_head_index which is still not used
 * INPUT    :None
 * OUTPUT   :None
 * RETURN   :the pointe the unused sharmem_data_p->md_head_index array element
 *-------------------------------------------------------------------------
 */
static I16_T CFM_OM_LocalGetFreeMdAllocation()
{
    UI32_T i;

    for(i=0; i< SYS_ADPT_CFM_MAX_NBR_OF_MD; i++)
    {
        if(FALSE == sharmem_data_p->md_ag[i].used)
            return i;
    }

    return -1;
}/*End of CFM_OM_LocalGetFreeMdAllocation*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_LocalGetFreeMaAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function find the element in sharmem_data_p->md_head_index which is still not used
 * INPUT    :None
 * OUTPUT   :None
 * RETURN   :the pointe the unused sharmem_data_p->md_head_index array element
 *-------------------------------------------------------------------------
 */
static I16_T CFM_OM_LocalGetFreeMaAllocation()
{
    I16_T i = -1;

    for(i=0; i< SYS_ADPT_CFM_MAX_NBR_OF_MA; i++)
    {
        if(FALSE == sharmem_data_p->ma_ag[i].used)
            return i;
    }

    return -1;
}/*End of CFM_OM_LocalGetFreeMaAllocation*/

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN != TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalCheckAllMdExistMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : this function will check the in the same level domain's ma  vid already exist
 *            in the vlan list or not if exist return true, or return fail
 * INPUT    : *ma_name_ap    - the ma name array pointer
 *            name_len       - the ma name length
 * OUTPUT   : None
 * RETURN   : TRUE         - the check ma name already exist in a md
 *            FALSE        - the check ma name not exist in a md
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalCheckAllMdExistMaName(
                                    UI8_T *ma_name_ap,
                                    UI32_T name_len)
{
    I16_T ma_ar_idx=-1;
    I16_T md_ar_idx=sharmem_data_p->md_head_index;

    /*check all md*/
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
             &&md_ar_idx >= 0)
    {
        ma_ar_idx=sharmem_data_p->md_ag[md_ar_idx].ma_start_ar_idx;

        /*check all ma under the md*/
        while(sharmem_data_p->ma_ag[ma_ar_idx].used
            &&ma_ar_idx >= 0)
        {
            if(name_len == sharmem_data_p->ma_ag[ma_ar_idx].name_length
              &&!memcmp(ma_name_ap, sharmem_data_p->ma_ag[ma_ar_idx].name_a, name_len) )
            {
                return TRUE;
            }
            ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }
    return FALSE;
}/*end of CFM_OM_LocalCheckAllMdExistMaName*/
#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN != TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalIsMaNameExistInMd
 *-------------------------------------------------------------------------
 * PURPOSE  : To check if ma name exists in specified md
 * INPUT    : md_p        - pointer to md to check
 *            ma_name_p   - pointer to ma name to check
 *            ma_name_len - ma name length
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalIsMaNameExistInMd(
    CFM_OM_MD_T *md_p,
    UI8_T       *ma_name_p,
    UI32_T      ma_name_len)
{
    I16_T   ma_ar_idx=-1;
    BOOL_T  ret = FALSE;

    if ((NULL != md_p) && (NULL != ma_name_p))
    {
        ma_ar_idx=md_p->ma_start_ar_idx;

        /*check all ma under the md*/
        while(sharmem_data_p->ma_ag[ma_ar_idx].used
            &&ma_ar_idx >= 0)
        {
            if (  (ma_name_len == sharmem_data_p->ma_ag[ma_ar_idx].name_length)
                &&(0 == memcmp(ma_name_p, sharmem_data_p->ma_ag[ma_ar_idx].name_a, ma_name_len))
               )
            {
                ret = TRUE;
                break;
            }
            ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
        }
    }

    return ret;
}/*end of CFM_OM_LocalIsMaNameExistInMd*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalIsMaNameExist
 *-------------------------------------------------------------------------
 * PURPOSE  : To check if ma name already exists.
 * INPUT    : md_p        - pointer to md to check
 *            ma_name_p   - pointer to ma name to check
 *            ma_name_len - ma name length
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalIsMaNameExist(
    CFM_OM_MD_T *md_p,
    UI8_T       *ma_name_p,
    UI32_T      ma_name_len)
{
    BOOL_T  ret;

#if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE)
    ret = CFM_OM_LocalIsMaNameExistInMd(md_p, ma_name_p, ma_name_len);
#else
    ret = CFM_OM_LocalCheckAllMdExistMaName(ma_name_p, ma_name_len);
#endif /* #if (SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN == TRUE) */

    return ret;
}/*end of CFM_OM_LocalIsMaNameExistInMd*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalCheckMaExistVid
 *-------------------------------------------------------------------------
 * PURPOSE  : this function will check the vid already exist in the vlan list or not
 *            if exist return true, or return fail
 * INPUT    : *ma_p       - the ma pointer
 *            checked_vid - the check vid list.
 * OUTPUT   : None
 * RETURN   : TRUE        - the check vid already exist in the vid list
 *            FALSE       - the check vid not exist in the vid list
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalCheckMaExistVid(
                                CFM_OM_MA_T *ma_p,
                                UI8_T checked_vid[(SYS_DFLT_DOT1QMAXVLANID/8)+1])
{
    UI16_T i;

    /*check already exit this vid or not*/
    for(i=0;i <(SYS_DFLT_DOT1QMAXVLANID/8)+1; i++)
        if(ma_p->vid_bitmap_a[i]&checked_vid[i])
            return TRUE;

    return FALSE;
}/*end of CFM_OM_LocalCheckMaExistVid*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalCheckAllMaExistVid
 *-------------------------------------------------------------------------
 * PURPOSE  : this function will check the in the same level domain's ma  vid already exist
 *            in the vlan list or not if exist return true, or return fail
 * INPUT    : *md_p        - the md pointer
 *            *ma_p        - the ma pointer
 *            *except_ma_p - the except ma pointer, which won;t check the exist vid
 *            checked_vid  - the check vid list.
 * OUTPUT   : None
 * RETURN   : TRUE         - the check vid already exist in the vid list
 *            FALSE        - the check vid not exist in the vid list
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalCheckAllMaExistVid(
                                CFM_OM_MD_T *md_p,
                                CFM_OM_MA_T *except_ma_p,
                                UI8_T checked_vid[(SYS_DFLT_DOT1QMAXVLANID/8)+1])
{
    I16_T md_ar_idx=sharmem_data_p->md_head_index, ma_ar_idx=-1;
    UI16_T i=0;

    /*check all md*/
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
          &&md_ar_idx >= 0)
    {
        if(sharmem_data_p->md_ag[md_ar_idx].level!= md_p->level)
        {
            md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
            continue;
        }

        ma_ar_idx=sharmem_data_p->md_ag[md_ar_idx].ma_start_ar_idx;

        /*check all ma under the md*/
        while(sharmem_data_p->ma_ag[ma_ar_idx].used
              &&ma_ar_idx >=0 )
        {
            if(except_ma_p!=NULL
            &&sharmem_data_p->ma_ag[ma_ar_idx].ar_idx== except_ma_p->ar_idx)
            {
                ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
                continue;
            }

            /*check already exit this vid or not*/
            for(i=0;i <(SYS_DFLT_DOT1QMAXVLANID/8)+1; i++)
                if(sharmem_data_p->ma_ag[ma_ar_idx].vid_bitmap_a[i]&checked_vid[i])
                    return TRUE;

            ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }
    return FALSE;
}/*end of CFM_OM_LocalCheckAllMaExistVid*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetLoopBackListHeadPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the loopback list head pointer
 * INPUT    : None
 * OUTPUT   : loopback list head pointer
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static CFM_OM_LoopBackListHead_T *CFM_OM_LocalGetLoopBackListHeadPtr()
{
    return &sharmem_data_p->loop_back_list_head_g;

}/*end of CFM_OM_LocalGetLoopBackListHeadPtr*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetError
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get unused array element
 * INPUT    : None
 * OUTPUT   : the pointe to the unused array element
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static I16_T CFM_OM_LocalGetFreeErrorAllocation()
{
    UI32_T i;

    for(i=0; i< SYS_ADPT_CFM_MAX_NBR_OF_ERROR; i++)
    {
        if(FALSE == sharmem_data_p->error_list_ag[i].used)
            return i;
    }

    return -1;
}/*End of CFM_OM_LocalGetFreeErrorAllocation*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetError
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the error element pointer
 * INPUT    : vid   - the vlan id
 *            lport - the logiacl port
 * OUTPUT   : **error_pp  - the error pointer
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetError(
                        UI16_T vid,
                        UI32_T lport,
                        CFM_OM_ErrorListElement_T **error_pp)
{
    CFM_OM_ErrorListHead_T *error_list_head = CFM_OM_LocalGetErrorListHeadPtr();
    I16_T cur_err_arx_idx=-1;

    if(-1 == error_list_head->first_ar_idx)
    {
        return FALSE;
    }

    cur_err_arx_idx=error_list_head->first_ar_idx;

    while(cur_err_arx_idx>=0
          &&TRUE == sharmem_data_p->error_list_ag[cur_err_arx_idx].used)
    {
        if((vid == sharmem_data_p->error_list_ag[cur_err_arx_idx].vid)
            &&(lport == sharmem_data_p->error_list_ag[cur_err_arx_idx].lport))
        {
            *error_pp=&sharmem_data_p->error_list_ag[cur_err_arx_idx];
            return TRUE;
        }
        cur_err_arx_idx=sharmem_data_p->error_list_ag[cur_err_arx_idx].nxt_ar_idx;
    }

    return FALSE;
}/*end of CFM_OM_LocalGetError*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalAddErrorsToErrorsList
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add the erorr to the error list
 * INPUT    : *error_p - the pointer to the next error
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalAddErrorsToErrorsList(
                                    CFM_OM_ErrorListElement_T *error_p)
{
    CFM_OM_ErrorListHead_T *error_list_head=CFM_OM_LocalGetErrorListHeadPtr();
    I16_T cur_err_ar_idx=-1, pre_err_ar_idx=-1;

    error_p->nxt_ar_idx=-1;

    /*there is no error, add to be the first node of the list*/
    if(-1 == error_list_head->first_ar_idx)
    {
        error_list_head->first_ar_idx=error_p->ar_idx;
        error_list_head->last_ar_idx=error_p->ar_idx;
        error_list_head->errors_number++;
        return TRUE;
    }

    cur_err_ar_idx=error_list_head->first_ar_idx;
    pre_err_ar_idx=cur_err_ar_idx;

    /*add in middle*/
    while(cur_err_ar_idx>=0
          &&TRUE == sharmem_data_p->error_list_ag[cur_err_ar_idx].used)
    {
        /*add before*/
        if(error_p->vid < sharmem_data_p->error_list_ag[cur_err_ar_idx].vid )
        {
            if(cur_err_ar_idx == pre_err_ar_idx)
            {
                error_p->nxt_ar_idx=error_list_head->first_ar_idx;
                error_list_head->first_ar_idx=error_p->ar_idx;
            }
            else
            {
                error_p->nxt_ar_idx=cur_err_ar_idx;
                sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=error_p->ar_idx;
            }
            error_list_head->errors_number++;
            return TRUE;
        }
        /*same vid, check lport*/
        else if (sharmem_data_p->error_list_ag[cur_err_ar_idx].vid == error_p->vid)
        {
            /*add before*/
            if(error_p->lport < sharmem_data_p->error_list_ag[cur_err_ar_idx].lport)
            {
                /*add first*/
                if(cur_err_ar_idx == pre_err_ar_idx)
                {
                    error_p->nxt_ar_idx=error_list_head->first_ar_idx;
                    error_list_head->first_ar_idx=error_p->ar_idx;
                }
                else /*add middle*/
                {
                    error_p->nxt_ar_idx=cur_err_ar_idx;
                    sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=error_p->ar_idx;
                }

                error_list_head->errors_number++;
                return TRUE;
            }
            /*same vid and lport, replace old*/
            else if(error_p->lport == sharmem_data_p->error_list_ag[cur_err_ar_idx].lport)
            {
                /*same vid and lport, replace old one, and update bitmap*/

                error_p->nxt_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
                sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx=-1;
                error_p->reason_bit_map|=sharmem_data_p->error_list_ag[cur_err_ar_idx].reason_bit_map;

                if(cur_err_ar_idx == pre_err_ar_idx)
                {
                    error_list_head->first_ar_idx=error_p->ar_idx;
                }
                else
                {
                    sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=error_p->ar_idx;
                }

                if(error_list_head->last_ar_idx == cur_err_ar_idx)
                {
                    error_list_head->last_ar_idx=error_p->ar_idx;
                }
                sharmem_data_p->error_list_ag[cur_err_ar_idx].used=FALSE;

                return TRUE;
            }
        }
        pre_err_ar_idx=cur_err_ar_idx;
        cur_err_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
    }

    /*add to the end*/
    if(pre_err_ar_idx == error_list_head->last_ar_idx)
    {
        sharmem_data_p->error_list_ag[error_list_head->last_ar_idx].nxt_ar_idx=error_p->ar_idx;
        error_list_head->last_ar_idx=error_p->ar_idx;
    }

    error_list_head->errors_number++;
    return TRUE;
}/*end of CFM_OM_LocalAddErrorsToErrorsList*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFreeLoopBackAllocation
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add the loopback element to the lbr list
 * INPUT    : *loopback_p - the pointer of the loop back
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static I16_T CFM_OM_GetFreeLoopBackAllocation()
 {
    UI32_T i;

    for(i=0; i< SYS_ADPT_CFM_MAX_NBR_OF_MA; i++)
    {
        if(FALSE == sharmem_data_p->lbr_list_ag[i].used)
        {
            return i;
        }
    }

    return -1;
 }/*End of CFM_OM_GetFreeLoopBackAllocation*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddToLoopBackList
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add the loopback element to the lbr list
 * INPUT    : *loopback_p - the pointer of the loop back
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_AddToLoopBackList(
                            CFM_OM_LoopBackListElement_T *loopback_p)
{
    CFM_OM_LoopBackListHead_T *loopback_list_head=CFM_OM_LocalGetLoopBackListHeadPtr();
    I16_T cur_lb_ar_idx=-1, pre_lb_ar_idx=-1;

    loopback_p->next_ar_idx=-1;

    /*add to the list*/
    if(-1 == loopback_list_head->first_ar_idx)
    {
        loopback_list_head->first_ar_idx=loopback_p->ar_idx;
        loopback_list_head->last_ar_idx=loopback_p->ar_idx;
        return TRUE;
    }

    /*add in middle*/
    cur_lb_ar_idx=loopback_list_head->first_ar_idx;
    pre_lb_ar_idx=loopback_list_head->first_ar_idx;

    while(-1!=cur_lb_ar_idx)
    {
        if(loopback_p->lbm_seq_number<sharmem_data_p->lbr_list_ag[cur_lb_ar_idx].lbm_seq_number)
        {
            if(loopback_list_head->first_ar_idx==cur_lb_ar_idx)
            {
                loopback_list_head->first_ar_idx=loopback_p->ar_idx;
                loopback_p->next_ar_idx=cur_lb_ar_idx;
                return TRUE;
            }
            else
            {
                sharmem_data_p->lbr_list_ag[pre_lb_ar_idx].next_ar_idx=loopback_p->ar_idx;
                loopback_p->next_ar_idx=cur_lb_ar_idx;
                return TRUE;
            }
        }

        pre_lb_ar_idx=cur_lb_ar_idx;
        cur_lb_ar_idx=sharmem_data_p->lbr_list_ag[cur_lb_ar_idx].next_ar_idx;
    }

    /*add in last*/
    if(pre_lb_ar_idx==loopback_list_head->last_ar_idx)
    {
        sharmem_data_p->lbr_list_ag[loopback_list_head->last_ar_idx].next_ar_idx=loopback_p->ar_idx;
        loopback_list_head->last_ar_idx=loopback_p->ar_idx;
    }
    return TRUE;
}/*end of CFM_OM_AddToLoopBackList*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalCheckAlreadyExistMdName
 * ------------------------------------------------------------------------
 * PURPOSE  : the function will check the name already exsit or not
 * INPUT    :*name_ap   - the md name array pointer
 *           name_length - the name length
 * OUTPUT   :
 * RETUEN   : TRUE  - success
 *            FALSE -fail.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalCheckAlreadyExistMdName(
                                    UI8_T *name_ap,
                                    UI32_T name_length)
{
    I16_T md_ar_idx=sharmem_data_p->md_head_index;

    if((NULL==name_ap )||(0== name_length))
    {
        return FALSE;
    }

    while( TRUE == sharmem_data_p->md_ag[md_ar_idx].used
             &&md_ar_idx >= 0)
    {
        if(name_length == sharmem_data_p->md_ag[md_ar_idx].name_length
           &&!memcmp(sharmem_data_p->md_ag[md_ar_idx].name_a, name_ap, name_length))
        {
            return TRUE;
        }

        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }

    return FALSE;
}/*End of CFM_OM_LocalCheckAlreadyExistMdName*/

 /* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMdByIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : md_index - md index
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static CFM_OM_MD_T* CFM_OM_LocalGetMdByIndex(
                                UI32_T md_index)
{
    I16_T md_ar_idx = sharmem_data_p->md_head_index;

    /*find the md*/
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
            &&md_ar_idx >= 0)
    {
        if(sharmem_data_p->md_ag[md_ar_idx].index == md_index)
        {
            return &sharmem_data_p->md_ag[md_ar_idx];
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }

    return NULL;
}/*end of CFM_OM_LocalGetMdByIndex*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMdByLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : level - the md level
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : this will get the first md in the same level
 * ------------------------------------------------------------------------
 */
static CFM_OM_MD_T* CFM_OM_LocalGetMdByLevel(
                                CFM_TYPE_MdLevel_T level)
{
    I16_T md_ar_idx = sharmem_data_p->md_head_index;

    /*find the md*/
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
         && md_ar_idx >= 0)
    {
        if(sharmem_data_p->md_ag[md_ar_idx].level== level)
        {
            return &sharmem_data_p->md_ag[md_ar_idx];
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }

    return NULL;
}/*end of CFM_OM_LocalGetMdByLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetNextMdByLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md according to the input level.
 * INPUT    : level - md level
 *            index - index of current md and to get the next md
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
*/
static CFM_OM_MD_T* CFM_OM_LocalGetNextMdByLevel(
                                    UI32_T md_index,
                                    CFM_TYPE_MdLevel_T level)
{
    I16_T md_ar_idx = sharmem_data_p->md_head_index;

    /*find the md*/
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
        && md_ar_idx >=0)
    {
        if((sharmem_data_p->md_ag[md_ar_idx].level == level)
            &&(md_index < sharmem_data_p->md_ag[md_ar_idx].index))
        {
            return &sharmem_data_p->md_ag[md_ar_idx];
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }

    return NULL;
}/*end of CFM_OM_LocalGetNextMdByLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMdByName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : *md_name_ap - md name
 *            name_len    - md name length
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    :
 * ------------------------------------------------------------------------
 */
static CFM_OM_MD_T* CFM_OM_LocalGetMdByName(
    UI8_T   *md_name_ap,
    UI32_T  name_len)
{
    I16_T md_ar_idx = sharmem_data_p->md_head_index;

    /*find the md*/
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
        &&md_ar_idx >= 0)
    {
        if(name_len == sharmem_data_p->md_ag[md_ar_idx].name_length
           &&!memcmp(md_name_ap, sharmem_data_p->md_ag[md_ar_idx].name_a, name_len))
        {
            return &sharmem_data_p->md_ag[md_ar_idx];
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }
    return NULL;
}/*end of CFM_OM_LocalGetMdByName*/

 /* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetNextMdByIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the next Md pointer
 * INPUT    : md_index - md index
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_LocalGetNextMdByIndex(
                                    UI32_T md_index)
{
    I16_T md_ar_idx = sharmem_data_p->md_head_index;

    /*find the md*/
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
            &&md_ar_idx >=0)
    {
        if(sharmem_data_p->md_ag[md_ar_idx].index > md_index)
        {
            return &sharmem_data_p->md_ag[md_ar_idx];
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }
    return NULL;
}/*end of CFM_OM_LocalGetNextMdByIndex*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMdMaByMdMaIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the pointer of md and ma according to
 *              the specified md and ma index.
 * INPUT    : md_index      - md index
 *            ma_index      - ma index
 * OUTPUT   : **return_md_p - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetMdMaByMdMaIndex(
    UI32_T          md_index,
    UI32_T          ma_index,
    CFM_OM_MD_T     **return_md_pp,
    CFM_OM_MA_T     **return_ma_pp)
{
    I16_T   ma_ar_idx=-1;
    I16_T   md_ar_idx=sharmem_data_p->md_head_index;

    while(md_ar_idx >= 0 &&
         TRUE == sharmem_data_p->md_ag[md_ar_idx].used)
    {
        ma_ar_idx=sharmem_data_p->md_ag[md_ar_idx].ma_start_ar_idx;
        while((ma_ar_idx >= 0)&&(sharmem_data_p->md_ag[md_ar_idx].index == md_index))
        {
            if (sharmem_data_p->ma_ag[ma_ar_idx].index==ma_index)
            {
                *return_md_pp=&sharmem_data_p->md_ag[md_ar_idx];
                *return_ma_pp=&sharmem_data_p->ma_ag[ma_ar_idx];
                return TRUE;
            }
            ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }
    return FALSE;
}/*end of CFM_OM_LocalGetMdMaByMdMaIndex*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMdMaByMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the input ma name.
 * INPUT    : *ma_name       - the ma name pointer
 *            name_len       - ma name length
 * OUTPUT   : **return_md_pp - the md record
 *            **return_ma_pp - the ma record
 * RETUEN   : TRUE           - success
 *            FALSE          - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetMdMaByMaName(
    UI8_T           *ma_name_ap,
    UI32_T          name_len,
    CFM_OM_MD_T     **return_md_pp,
    CFM_OM_MA_T     **return_ma_pp)
{
    I16_T   ma_ar_idx=-1;
    I16_T   md_ar_idx=sharmem_data_p->md_head_index;

    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
        &&md_ar_idx >= 0)
    {
        ma_ar_idx=sharmem_data_p->md_ag[md_ar_idx].ma_start_ar_idx;
        while(ma_ar_idx >= 0)
        {
            if (name_len == sharmem_data_p->ma_ag[ma_ar_idx].name_length
                &&!memcmp(sharmem_data_p->ma_ag[ma_ar_idx].name_a, ma_name_ap, name_len))
            {
                *return_md_pp=&sharmem_data_p->md_ag[md_ar_idx];
                *return_ma_pp=&sharmem_data_p->ma_ag[ma_ar_idx];
                return TRUE;
            }
            ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }
    return FALSE;
}/*end of CFM_OM_LocalGetMdMaByMaName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMaByName
 * ------------------------------------------------------------------------
 * PURPOSE  : To get pointer of ma for specified ma name and md
 * INPUT    : ma_name_p   - ma name
 *            ma_name_len - md name length
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    :
 * ------------------------------------------------------------------------
 */
static CFM_OM_MA_T* CFM_OM_LocalGetMaByName(
    UI8_T       *ma_name_p,
    UI32_T      ma_name_len,
    CFM_OM_MD_T *md_p)
{
    I16_T       ma_ar_idx = -1;
    CFM_OM_MA_T *ret_p = NULL;

    ma_ar_idx = md_p->ma_start_ar_idx;
    while(ma_ar_idx >= 0)
    {
        if (  (ma_name_len == sharmem_data_p->ma_ag[ma_ar_idx].name_length)
            &&(0 == memcmp(sharmem_data_p->ma_ag[ma_ar_idx].name_a, ma_name_p, ma_name_len))
           )
        {
            ret_p = &sharmem_data_p->ma_ag[ma_ar_idx];
            break;
        }
        ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }

    return ret_p;
}/*end of CFM_OM_LocalGetMdByName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMdMaByMdMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : To get pointer of md/ma for specified md and ma name.
 * INPUT    : md_name_p    - the ma name pointer
 *            md_name_len  - ma name length
 *            ma_name_p    - the ma name pointer
 *            ma_name_len  - ma name length
 * OUTPUT   : return_md_pp - the md record
 *            return_ma_pp - the ma record
 * RETUEN   : TRUE/FALSE
 * NOTES    : Use ma_name only to find the md/ma if md_name_len == 0
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetMdMaByMdMaName(
    UI8_T           *md_name_p,
    UI32_T          md_name_len,
    UI8_T           *ma_name_p,
    UI32_T          ma_name_len,
    CFM_OM_MD_T     **return_md_pp,
    CFM_OM_MA_T     **return_ma_pp)
{
    CFM_OM_MD_T *md_p = NULL;
    CFM_OM_MA_T *ma_p = NULL;
    BOOL_T      ret = FALSE;

    if (md_name_len != 0)
    {
        md_p = CFM_OM_LocalGetMdByName(md_name_p, md_name_len);
        if (NULL != md_p)
        {
            ma_p = CFM_OM_LocalGetMaByName(ma_name_p, ma_name_len, md_p);
        }
    }
    else
    {
        I16_T   md_ar_idx=sharmem_data_p->md_head_index;

        while (   (TRUE == sharmem_data_p->md_ag[md_ar_idx].used)
                &&(md_ar_idx >= 0)
              )
        {
            md_p = &sharmem_data_p->md_ag[md_ar_idx];
            ma_p = CFM_OM_LocalGetMaByName(ma_name_p, ma_name_len, md_p);

            if (NULL != ma_p)
            {
                break;
            }
            else
            {
                md_ar_idx = sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
            }
        }
    }

    if ((NULL != md_p) && (NULL != ma_p))
    {
        *return_md_pp = md_p;
        *return_ma_pp = ma_p;
        ret = TRUE;
    }

    return ret;
}/*end of CFM_OM_LocalGetMdMaByMaName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMdMaByMdIndxMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the input ma name.
 * INPUT    : md_index       - the md index
 *            *ma_name_ap    - the ma name array pointer
 *            name_len       - the ma name length
 * OUTPUT   : **return_md_pp - the md record
 *            **return_ma_pp - the ma record
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetMdMaByMdIndxMaName(
                                UI32_T md_index,
                                UI8_T *ma_name_ap,
                                UI32_T name_len,
                                CFM_OM_MD_T **return_md_pp,
                                CFM_OM_MA_T **return_ma_pp)
{
    CFM_OM_MD_T *md_p =NULL;
    I16_T  ma_ar_idx=-1;

    md_p=CFM_OM_LocalGetMdByIndex( md_index);

    if(NULL == md_p)
    {
        return FALSE;
    }

    ma_ar_idx=md_p->ma_start_ar_idx;

    while(ma_ar_idx >= 0)
    {
        if (name_len == sharmem_data_p->ma_ag[ma_ar_idx].name_length
            &&!memcmp(sharmem_data_p->ma_ag[ma_ar_idx].name_a, ma_name_ap, name_len)
            )
        {
            *return_ma_pp=&sharmem_data_p->ma_ag[ma_ar_idx];
            *return_md_pp=md_p;
            return TRUE;
        }

        ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }

    return FALSE;
}/*end of CFM_OM_LocalGetMdMaByMdIndxMaName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMdMaByLevelVid
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the level vid
 * INPUT    : level         - the md level
 *            vid           - the vlan id
 * OUTPUT   : *return_md_p  - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetMdMaByLevelVid(
                            CFM_TYPE_MdLevel_T level,
                            UI16_T vid,
                            CFM_OM_MD_T **return_md_pp,
                            CFM_OM_MA_T **return_ma_pp)
{
    I16_T md_ar_idx = sharmem_data_p->md_head_index;
    I16_T ma_ar_idx=-1;
    UI8_T checked_vid [ (SYS_DFLT_DOT1QMAXVLANID/8)+1 ]={0};

    if ((NULL == return_md_pp) || (NULL == return_ma_pp) ||
        (0    == vid) || (vid > SYS_DFLT_DOT1QMAXVLANID))
    {
        return FALSE;
    }

    checked_vid[((vid-1)/8)]|=(1 << (7 - ((vid-1)%8)));
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
        &&md_ar_idx >= 0)
    {
        if(sharmem_data_p->md_ag[md_ar_idx].level != level)
        {
            md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
            continue;
        }

        ma_ar_idx=sharmem_data_p->md_ag[md_ar_idx].ma_start_ar_idx;

        while(ma_ar_idx>=0
            &&TRUE == sharmem_data_p->ma_ag[ma_ar_idx].used)
        {
            if(TRUE == CFM_OM_LocalCheckMaExistVid(&sharmem_data_p->ma_ag[ma_ar_idx], checked_vid))
            {
                *return_md_pp=&sharmem_data_p->md_ag[md_ar_idx];
                *return_ma_pp=&sharmem_data_p->ma_ag[ma_ar_idx];
                return TRUE;
            }
            ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;

    }

    return FALSE;
}/*end of CFM_OM_LocalGetMdMaByLevelVid*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetFirstHighLevelMdMaBylevelVid
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the first low level md and ma index according to the level vid
 * INPUT    : level         - the md level
 *            vid           - the vlan id
 * OUTPUT   : *return_md_p  - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetFirstHighLevelMdMaBylevelVid(
                                CFM_TYPE_MdLevel_T level,
                                UI16_T vid,
                                CFM_OM_MD_T **return_md_pp,
                                CFM_OM_MA_T **return_ma_pp)
{
    I16_T md_ar_idx = sharmem_data_p->md_head_index, ma_ar_idx=-1;
    UI16_T min_diff_level=0xffff;
    UI8_T checked_vid [ (SYS_DFLT_DOT1QMAXVLANID/8)+1 ]={0};
    BOOL_T finded=FALSE;

    checked_vid[(UI32_T)((vid-1)/8)]|=(1 << (7 - ((vid-1)%8)));

    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
        &&md_ar_idx >= 0)
    {
        if(sharmem_data_p->md_ag[md_ar_idx].level <= level)
        {
            md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
            continue;
        }

        ma_ar_idx=sharmem_data_p->md_ag[md_ar_idx].ma_start_ar_idx;

        while(ma_ar_idx>=0
            &&TRUE == sharmem_data_p->ma_ag[ma_ar_idx].used)
        {
            if(TRUE == CFM_OM_LocalCheckMaExistVid(&sharmem_data_p->ma_ag[ma_ar_idx], checked_vid))
            {
                if((sharmem_data_p->md_ag[md_ar_idx].level - level)<min_diff_level)
                {
                    *return_md_pp=&sharmem_data_p->md_ag[md_ar_idx];
                    *return_ma_pp=&sharmem_data_p->ma_ag[ma_ar_idx];

                    finded=TRUE;
                    min_diff_level=(sharmem_data_p->md_ag[md_ar_idx].level-level);
                }
            }
            ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
        }
        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }

    if(TRUE == finded )
    {
        return TRUE;
    }

    return FALSE;
}/*end of CFM_OM_LocalGetFirstHighLevelMdMaBylevelVid*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetNextMaByMaIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the next ma pointer
 * INPUT    : md_index - md index
 *            ma_index - ma index
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    : This get next ma is get the next ma under the md
 * ------------------------------------------------------------------------
 */
static CFM_OM_MA_T* CFM_OM_LocalGetNextMaByMaIndex(
                                    UI32_T md_index,
                                    UI32_T ma_index)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    I16_T ma_ar_idx=-1;

    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        /*md doesn't exist*/
        return NULL;
    }

    /*find the ma*/
    ma_ar_idx=md_p->ma_start_ar_idx;

    while(TRUE == sharmem_data_p->ma_ag[ma_ar_idx].used
        &&ma_ar_idx >= 0)
    {
        if(sharmem_data_p->ma_ag[ma_ar_idx].index > ma_index)
        {
            ma_p = &sharmem_data_p->ma_ag[ma_ar_idx];
            return ma_p;
        }

        ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }

    return NULL;
}/*end of CFM_OM_LocalGetNextMaByMaIndex*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetNextMaByMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get  the next ma pointer
 * INPUT    : md_index    - the md index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    : This get next ma is get the next ma under the md
 * ------------------------------------------------------------------------
 */
static CFM_OM_MA_T* CFM_OM_LocalGetNextMaByMaName(
                                    UI32_T md_index,
                                    UI8_T *ma_name_ap,
                                    UI32_T name_len)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    I16_T ma_ar_idx=-1;

    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        /*md doesn't exist*/
        return NULL;
    }

    /*find the ma*/
    ma_ar_idx=md_p->ma_start_ar_idx;

    while(ma_ar_idx >= 0)
    {
        if(sharmem_data_p->ma_ag[ma_ar_idx].name_length == name_len
           &&!memcmp(sharmem_data_p->ma_ag[ma_ar_idx].name_a, ma_name_ap, name_len))
        {
            ma_p=&sharmem_data_p->ma_ag[ma_ar_idx];
            return ma_p;
        }
        ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }
    return NULL;
}/*end of CFM_OM_LocalGetNextMaByMaName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma content
 * INPUT    : md_index - md index
 *            ma_index - ma index
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static CFM_OM_MA_T* CFM_OM_LocalGetMa(
                        UI32_T md_index,
                        UI32_T ma_index)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    I16_T ma_ar_idx=-1;

    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        /*md doesn't exist*/
        return NULL;
    }

    /*find the ma*/
    ma_ar_idx=md_p->ma_start_ar_idx;

    while(ma_ar_idx >= 0)
    {
        if(sharmem_data_p->ma_ag[ma_ar_idx].index == ma_index)
        {
            ma_p = &sharmem_data_p->ma_ag[ma_ar_idx];
            return ma_p;
        }

        ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }
    return NULL;
}/*end of CFM_OM_LocalGetMa*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_LocalFindNextMdIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function find  the next md index
 * INPUT    :
 * OUTPUT   :
 * RETURN   : the next md index can use
 * NOTE     : 1.if the input md index not the same as next_md_index,
 *           md_index needn't find next.
 *-------------------------------------------------------------------------
 */
static UI32_T CFM_OM_LocalFindNextMdIndex(
                                    UI32_T current_md_index)
{
    CFM_OM_MD_T *md_p=NULL;
    UI32_T nxt_md_index=sharmem_data_p->md_next_index_g ;

    if(current_md_index != sharmem_data_p->md_next_index_g )
    {
        return sharmem_data_p->md_next_index_g;
    }

    while(NULL!=(md_p=CFM_OM_LocalGetMdByIndex(nxt_md_index)))
    {
        nxt_md_index=md_p->index+1;
    }
    sharmem_data_p->md_next_index_g =nxt_md_index;
    return nxt_md_index;
}/*End of CFM_OM_LocalFindNextMdIndex*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_LocalFindNextMaIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function find  the next ma index
 * INPUT    :
 * OUTPUT   :
 * RETURN   : the next ma index can use
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static UI32_T CFM_OM_LocalFindNextMaIndex(
                            UI32_T md_index,
                            UI32_T current_ma_index)
{
    CFM_OM_MA_T *ma_p=NULL;
    UI32_T nxt_ma_index=current_ma_index+1;

    while(NULL!=(ma_p=CFM_OM_LocalGetMa(md_index, nxt_ma_index)))
    {
        nxt_ma_index=nxt_ma_index+1;
    }

    return nxt_ma_index;
}/*End of CFM_OM_LocalFindNextMaIndex*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetNextError
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the next error
 * INPUT    : vid  - the vlan id
 *            lport- the logical port
 * OUTPUT   :**error_p - the next error
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : if *error_pp is NULL, then return the fisrt element in list
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalGetNextError(
                            UI16_T vid,
                            UI32_T lport,
                            CFM_OM_ErrorListElement_T **error_pp)
{
    CFM_OM_ErrorListHead_T *error_list_head = CFM_OM_LocalGetErrorListHeadPtr();
    I16_T cur_err_ar_idx=-1;

    cur_err_ar_idx=error_list_head->first_ar_idx;
    while(cur_err_ar_idx >= 0
        &&TRUE == sharmem_data_p->error_list_ag[cur_err_ar_idx].used)
    {
        if(    (vid < sharmem_data_p->error_list_ag[cur_err_ar_idx].vid)
            || (   (vid == sharmem_data_p->error_list_ag[cur_err_ar_idx].vid)
                 &&(lport < sharmem_data_p->error_list_ag[cur_err_ar_idx].lport)
               )
          )
        {
            *error_pp=&sharmem_data_p->error_list_ag[cur_err_ar_idx];
            return TRUE;
        }

        cur_err_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
    }
    return FALSE;
}/*end of CFM_OM_LocalGetNextError*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalFillMdInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : This function fill the md infomation
 * INPUT    : *md_p - the md pointer
 * OUTPUT   :*md_info_p - the md info pointer
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void CFM_OM_LocalFillMdInfo(
                        CFM_OM_MD_T *md_p,
                        CFM_OM_MdInfo_T *md_info_p)
{
    memset(md_info_p, 0, sizeof(CFM_OM_MdInfo_T));

    md_info_p->index=md_p->index;
    md_info_p->level=md_p->level;
    md_info_p->mep_archive_hold_time=md_p->mep_achive_holdtime/60;
    memcpy(md_info_p->name_a, md_p->name_a, md_p->name_length);
    md_info_p->name_len = md_p->name_length;
    md_info_p->name_format=md_p->format;
    md_info_p->fng_alarm_time=md_p->fng_alarm_time;
    md_info_p->fng_reset_time=md_p->fng_reset_time;
    md_info_p->lowest_alarm_pri=md_p->low_pri_def;
    md_info_p->mhf_creation=md_p->mhf_creation;
    md_info_p->permission=md_p->mhf_id_permission;
    md_info_p->next_index=md_p->next_available_ma_index;
    md_info_p->row_status=VAL_dot1agCfmMdRowStatus_active;
}/*End of CFM_OM_LocalFillMdInfo*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalFillMaInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : This function fill the md infomation
 * INPUT    : *ma_p - the md pointer
 * OUTPUT   : *ma_info_p - the md info pointer
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void CFM_OM_LocalFillMaInfo(
                        CFM_OM_MA_T *ma_p,
                        CFM_OM_MaInfo_T *ma_info_p)
{
    memset(ma_info_p, 0, sizeof(CFM_OM_MaInfo_T));

    ma_info_p->ma_index= ma_p->index;
    memcpy(ma_info_p->ma_name_a, ma_p->name_a, ma_p->name_length);
    ma_info_p->ma_name_len = ma_p->name_length;
    ma_info_p->md_index = sharmem_data_p->md_ag[ma_p->md_ar_idx].index;
    memcpy(ma_info_p->md_name_a, sharmem_data_p->md_ag[ma_p->md_ar_idx].name_a, sharmem_data_p->md_ag[ma_p->md_ar_idx].name_length);
    ma_info_p->md_name_len = sharmem_data_p->md_ag[ma_p->md_ar_idx].name_length;
    ma_info_p->name_format=ma_p->format;
    ma_info_p->md_level = sharmem_data_p->md_ag[ma_p->md_ar_idx].level;

    memcpy(ma_info_p->vlan_list_a, ma_p->vid_bitmap_a, (SYS_DFLT_DOT1QMAXVLANID/8)+1);

    ma_info_p->num_of_vids = ma_p->num_of_vids;
    ma_info_p->primary_vid = ma_p->primary_vid;
    ma_info_p->interval = ma_p->ccm_interval;
    ma_info_p->mhf_creation=ma_p->mhf_creation;
    ma_info_p->permission=ma_p->mhf_id_permission;
    ma_info_p->ccm_status = ma_p->ccm_status;
    ma_info_p->ais_level = ma_p->ais_send_level;
    ma_info_p->ais_period = ma_p->ais_period;
    ma_info_p->ais_status = ma_p->ais_status;
    ma_info_p->ais_suppress_status = ma_p->ais_supress_status;

    /* ais_suppresing == TRUE if
     *   AIS received && suppress alarm enabled && crosscheck enabled
     */
    ma_info_p->ais_suppresing =
        (  (-1   != ma_p->ais_rcvd_timer_idx)
         &&(CFM_TYPE_AIS_STATUS_ENABLE == ma_p->ais_supress_status)
         &&(CFM_TYPE_CROSS_CHECK_STATUS_ENABLE == ma_p->cross_check_status)
        ) ? TRUE : FALSE;

    ma_info_p->row_status=VAL_dot1agCfmMaRowStatus_active;
}/*End of CFM_OM_LocalFillMaInfo*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalFillMepInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : This function fill the mep infomation
 * INPUT    : *mep_p - the mep pointer
 * OUTPUT   : *mep_info_p - the mep info pointer
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalFillMepInfo(
    CFM_OM_MEP_T        *mep_p,
    CFM_OM_MepInfo_T    *mep_info_p)
{
    memset(mep_info_p, 0, sizeof(CFM_OM_MepInfo_T));

    mep_info_p->md_index = sharmem_data_p->md_ag[mep_p->md_ar_idx].index;
    mep_info_p->md_level = sharmem_data_p->md_ag[mep_p->md_ar_idx].level;
    memcpy(mep_info_p->md_name_a, sharmem_data_p->md_ag[mep_p->md_ar_idx].name_a, sharmem_data_p->md_ag[mep_p->md_ar_idx].name_length);
    mep_info_p->ma_index=sharmem_data_p->ma_ag[mep_p->ma_ar_idx].index;
    memcpy(mep_info_p->ma_name_a, sharmem_data_p->ma_ag[mep_p->ma_ar_idx].name_a, sharmem_data_p->ma_ag[mep_p->ma_ar_idx].name_length);
    mep_info_p->md_name_len = sharmem_data_p->md_ag[mep_p->md_ar_idx].name_length;
    mep_info_p->ma_name_len = sharmem_data_p->ma_ag[mep_p->ma_ar_idx].name_length;
    memcpy(mep_info_p->mac_addr_a, mep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN);
    mep_info_p->direction=mep_p->direction;
    mep_info_p->identifier=mep_p->identifier;
    mep_info_p->ccm_status=mep_p->cci_status;
    mep_info_p->lport=mep_p->lport;
    mep_info_p->primary_vid=mep_p->primary_vid;
    mep_info_p->active=mep_p->active;
    mep_info_p->fng_state=mep_p->fng_machine_state;
    mep_info_p->highest_pri_defect=mep_p->highest_pri_defect;
    mep_info_p->low_pri_def=mep_p->lowest_priority_defect;

    mep_info_p->cur_highest_pri_defect=CFM_OM_GetCurrentMaxErrPri(mep_p, FALSE);

    mep_info_p->fng_reset_time=mep_p->fng_reset_time;
    mep_info_p->fng_alarm_time=mep_p->fng_alarm_time;
    if(TRUE == mep_p->some_rdi_defect)
    {
        mep_info_p->defects|=1;
    }
    if(TRUE == mep_p->err_mac_status)
    {
        mep_info_p->defects|=(1<<1);
    }
    if(TRUE == mep_p->some_rmep_ccm_defect)
    {
        mep_info_p->defects|=(1<<2);
    }
    if(TRUE == mep_p->error_ccm_defect)
    {
        mep_info_p->defects|=(1<<3);
    }
    if(TRUE == mep_p->xcon_ccm_defect)
    {
        mep_info_p->defects|=(1<<4);
    }
    mep_info_p->ccm_ltm_priority=mep_p->ccm_ltm_priority;
    mep_info_p->ccm_seq_error=mep_p->rcvd_ccm_sequenc_errors;
    mep_info_p->cci_sent_ccms=mep_p->cci_sent_ccms;
    /*lbm lbr*/
    mep_info_p->lbr_out=mep_p->lbr_out;
    mep_info_p->lbr_in=mep_p->lbr_in;
    mep_info_p->lbr_in_out_of_order=mep_p->lbr_in_out_of_order;
    mep_info_p->lbr_bad_msdu=mep_p->lbr_bad_msdu;
    mep_info_p->next_lbm_trans_id=mep_p->next_lbm_trans_id;
    mep_info_p->trans_lbm_seq_num=mep_p->transmit_lbm_seq_number;
    memcpy(mep_info_p->trans_lbm_dst_mac_addr_a, mep_p->transmit_lbm_dest_mac_address_a, SYS_ADPT_MAC_ADDR_LEN);
    mep_info_p->trans_lbm_dst_mep_id=mep_p->transmit_lbm_dest_mep_id;
    mep_info_p->trans_lbm_dst_is_mep_id=mep_p->transmit_lbm_dest_is_mep_id;
    mep_info_p->trans_lbm_msg=mep_p->transmit_lbm_messages;
    mep_info_p->unexp_ltr_in=mep_p->unexp_ltr_in;
    mep_info_p->ltm_next_seq_num=mep_p->ltm_next_seq_number;
    mep_info_p->trans_lbm_vlan_priority=mep_p->transmit_lbm_vlan_priority ;
    mep_info_p->trans_lbm_vlan_drop_enabled=mep_p->transmit_lbm_vlan_drop_enable;
    mep_info_p->trans_lbm_result_ok=mep_p->transmit_lbm_result_oK;
    mep_info_p->trans_lbm_status=mep_p->tramsmit_lbm_status;
    /*ltm*/
    memcpy(mep_info_p->trans_ltm_target_mac_addr_a, mep_p->transmit_ltm_target_mac_address_a, SYS_ADPT_MAC_ADDR_LEN);
    mep_info_p->trans_ltm_target_mep_id=mep_p->transmit_ltm_target_mep_id;
    mep_info_p->trans_ltm_target_is_mep_id=mep_p->transmit_ltm_target_is_mep_id;
    mep_info_p->trans_ltm_ttl=mep_p->transmit_ltm_ttl;
    mep_info_p->trans_ltm_result=mep_p->transmit_ltm_result;
    mep_info_p->trans_ltm_seq_num=mep_p->transmit_ltm_seq_number;
    memcpy(mep_info_p->trans_ltm_egress_id_a, mep_p->transmit_ltm_egress_identifier, SYS_ADPT_MAC_ADDR_LEN+2);
    mep_info_p->ltm_use_fdb_only=mep_p->ltm_use_fdb_only;
    mep_info_p->trans_ltm_status=mep_p->transmit_ltm_status;
    mep_info_p->row_status=VAL_dot1agCfmMepRowStatus_active;

    return TRUE;
}/*End of CFM_OM_LocalFillMepInfo*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_LocalFillRemoteMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function fill the remote mep info
 * INPUT    : *rmep_p  - the content of remote mep
 * OUTPUT   : *rmep_info_p - the remtoe mep info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static void CFM_OM_LocalFillRemoteMepInfo(
    CFM_OM_REMOTE_MEP_T             *rmep_p,
    CFM_OM_RemoteMepCrossCheck_T    *rmep_info_p,
    UI32_T                          current_time)
{
    memset(rmep_info_p, 0, sizeof(CFM_OM_RemoteMepCrossCheck_T));

    strncpy((char *) rmep_info_p->md_name_a, (char *)sharmem_data_p->md_ag[rmep_p->md_ar_idx].name_a, sharmem_data_p->md_ag[rmep_p->md_ar_idx].name_length);
    strncpy((char *) rmep_info_p->ma_name_a, (char *)sharmem_data_p->ma_ag[rmep_p->ma_ar_idx].name_a, sharmem_data_p->ma_ag[rmep_p->ma_ar_idx].name_length);
    rmep_info_p->md_name_len = sharmem_data_p->md_ag[rmep_p->md_ar_idx].name_length;
    rmep_info_p->ma_name_len = sharmem_data_p->ma_ag[rmep_p->ma_ar_idx].name_length;
    rmep_info_p->level        =sharmem_data_p->md_ag[rmep_p->md_ar_idx].level;
    rmep_info_p->mep_id        =rmep_p->identifier;

    rmep_info_p->mep_up        =rmep_p->mep_up;
    rmep_info_p->primary_vid   =sharmem_data_p->ma_ag[rmep_p->ma_ar_idx].primary_vid;
    rmep_info_p->incoming_port = rmep_p->rcvd_lport;
    rmep_info_p->cc_life_time   =rmep_p->cc_life_time;
    current_time/=100; /*chang to sec from tick*/
    rmep_info_p->age_of_last_cc  =rmep_p->age_out<current_time?0:rmep_p->age_out - current_time;
    rmep_info_p->frame_loss    =rmep_p->frame_loss;
    rmep_info_p->packet_rcvd_count=rmep_p->packet_received;
    rmep_info_p->packet_error_count   =rmep_p->packet_error;
    memcpy(rmep_info_p->mep_mac_a,rmep_p->mac_addr_a, SYS_ADPT_MAC_ADDR_LEN);

    rmep_info_p->cross_check_enabled=(CFM_TYPE_CROSS_CHECK_STATUS_ENABLE==sharmem_data_p->ma_ag[rmep_p->ma_ar_idx].cross_check_status?TRUE:FALSE);

    rmep_info_p->port_status      = rmep_p->port_status;
    rmep_info_p->interface_status = rmep_p->interface_status;
    rmep_info_p->rdi=rmep_p->rdi;
    rmep_info_p->mep_state=rmep_p->machine_state;
    rmep_info_p->failed_ok_time=rmep_p->failed_ok_time;

    rmep_info_p->chassis_id_subtype=rmep_p->sender_chassis_id_sub_type;
    rmep_info_p->chassis_id_len=rmep_p->sender_chassis_id_length;

    if(0!=rmep_info_p->chassis_id_len)
    {
        memcpy(rmep_info_p->chassis_id_a, rmep_p->sender_chassis_id, rmep_p->sender_chassis_id_length);
    }

    if(0!=rmep_p->man_domain_length)
    {
        rmep_info_p->mgmt_addr_domain_len = CFM_TYPE_MAX_MAN_DOMAIN_LENGTH;
        /* for snmp access, need to convert back to the oid form
         */
        CFM_OM_CnvDataToOid(
            rmep_p->man_domain_address,
            rmep_p->man_domain_length,
            (UI32_T *) rmep_info_p->mgmt_addr_domain_a,
            &rmep_info_p->mgmt_addr_domain_len);
    }

    rmep_info_p->mgmt_len=rmep_p->man_length;

    if(0!=rmep_info_p->mgmt_len)
    {
        memcpy(rmep_info_p->mgmt_addr_a, rmep_p->man_address, rmep_p->man_length);
    }

    rmep_info_p->row_status = VAL_dot1agCfmMaMepListRowStatus_active;
}/*End of CFM_OM_LocalFillRemoteMepInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalFillLinktraceReplyInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function fill the linktrace information
 * INPUT    : *ltr_p        - the ltr conent
 * OUTPUT   : *ltr_info_p   - the ltr info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalFillLinktraceReplyInfo(
    CFM_OM_LTR_T            *ltr_p,
    CFM_OM_LinktraceReply_T *ltr_info_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;

    {
        UI8_T   *org_spec_tlv_bkp_p;

        /* need to backup the pointer to buffer of org_specific_tlv
         */
        org_spec_tlv_bkp_p = ltr_info_p->org_specific_tlv_p;
        memset(ltr_info_p, 0, sizeof(CFM_OM_LinktraceReply_T));
        ltr_info_p->org_specific_tlv_p = org_spec_tlv_bkp_p;
    }

    if (TRUE ==CFM_OM_LocalGetMdMaByMdMaIndex(ltr_p->md_index, ltr_p->ma_index, &md_p, &ma_p))
    {
        memcpy(ltr_info_p->md_name_a, md_p->name_a, md_p->name_length);
        memcpy(ltr_info_p->ma_name_a, ma_p->name_a, ma_p->name_length);
        ltr_info_p->md_name_len = md_p->name_length;
        ltr_info_p->ma_name_len = ma_p->name_length;
    }

    ltr_info_p->hops=ltr_p->receive_order;
    ltr_info_p->reply_ttl=ltr_p->reply_ttl;
    ltr_info_p->forwarded =ltr_p->forwarded;
    ltr_info_p->terminal_mep=ltr_p->terminal_mep;
    ltr_info_p->chassis_id_subtype = ltr_p->chassis_id_subtype;
    ltr_info_p->chassis_id_len=ltr_p->chassis_id_length;
    if (ltr_info_p->chassis_id_len>CFM_TYPE_MAX_CHASSIS_ID_LENGTH)
    {
        ltr_info_p->chassis_id_len=CFM_TYPE_MAX_CHASSIS_ID_LENGTH;
    }
    memcpy(ltr_info_p->chassis_id_a, ltr_p->chassis_id, ltr_info_p->chassis_id_len);
    memcpy(ltr_info_p->last_egress_id_a, ltr_p->last_egress_identifier, SYS_ADPT_MAC_ADDR_LEN +2);
    memcpy(ltr_info_p->next_egress_id_a, ltr_p->next_egress_identifier, SYS_ADPT_MAC_ADDR_LEN +2);

    if (0 != ltr_p->mgmt_addr_domain_len)
    {
        ltr_info_p->mgmt_domain_len = CFM_TYPE_MAX_MAN_DOMAIN_LENGTH;
        /* for snmp access, need to convert back to the oid form
         */
        CFM_OM_CnvDataToOid(
            ltr_p->mgmt_addr_domain,
            ltr_p->mgmt_addr_domain_len,
            (UI32_T *) ltr_info_p->mgmt_domain_a,
            &ltr_info_p->mgmt_domain_len);

    }

    ltr_info_p->mgmt_addr_len=(ltr_p->mgmt_addr_len>CFM_TYPE_MAX_MAN_ADDRESS_LENGTH?CFM_TYPE_MAX_MAN_ADDRESS_LENGTH:ltr_p->mgmt_addr_len);
    memcpy(ltr_info_p->mgmt_addr_a, ltr_p->mgmt_addr, ltr_info_p->mgmt_addr_len);

    ltr_info_p->relay_action=ltr_p->relay_action;

    ltr_info_p->ingress_action=ltr_p->ingress_action;
    memcpy(ltr_info_p->ingress_port_mac_a,ltr_p->ingress_mac, SYS_ADPT_MAC_ADDR_LEN);
    ltr_info_p->ingress_port_id_subtype=ltr_p->ingress_port_id_subtype;
    ltr_info_p->ingress_port_id_len=ltr_p->ingress_port_id_lenth;
    memcpy(ltr_info_p->ingress_port_id_a, ltr_p->ingress_port_Id, ltr_p->ingress_port_id_lenth);


    ltr_info_p->egress_action=ltr_p->egress_action;
    memcpy(ltr_info_p->egress_port_mac_a,ltr_p->egress_mac, SYS_ADPT_MAC_ADDR_LEN);
    ltr_info_p->egress_port_id_subtype=ltr_p->egress_port_id_subtype;
    ltr_info_p->egress_port_id_len=ltr_p->egress_port_id_lenth;
    memcpy(ltr_info_p->egress_port_id_a, ltr_p->egress_port_id, ltr_p->egress_port_id_lenth);

    if (NULL!=ltr_info_p->org_specific_tlv_p)
    {
        ltr_info_p->org_specific_tlv_len=(ltr_p->org_tlv_length>CFM_TYPE_MAX_ORGANIZATION_TLV_LENGTH?CFM_TYPE_MAX_ORGANIZATION_TLV_LENGTH:ltr_p->org_tlv_length);
        memcpy(ltr_info_p->org_specific_tlv_p, ltr_p->org_specific_tlv_a, ltr_info_p->org_specific_tlv_len);
    }

    return TRUE;
}/*End of CFM_OM_LocalFillLinktraceReplyInfo*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_LocalFillErrorInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function fill in the error info
 * INPUT    :*error_om_p  - the error information from om
 * OUTPUT   : *error_info - the error info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :move to om
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_LocalFillErrorInfo(
                        CFM_OM_ErrorListElement_T *error_om_p,
                        CFM_OM_Error_T *error_info_p)
{
    memset(error_info_p, 0, sizeof(CFM_OM_Error_T));

    memcpy(error_info_p->mac_addr_a, error_om_p->mac_a, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(error_info_p->md_name_a, sharmem_data_p->md_ag[error_om_p->md_ar_idx].name_a,
                  sharmem_data_p->md_ag[error_om_p->md_ar_idx].name_length);
    memcpy(error_info_p->ma_name_a, sharmem_data_p->ma_ag[error_om_p->ma_ar_idx].name_a,
                sharmem_data_p->ma_ag[error_om_p->ma_ar_idx].name_length);
    error_info_p->level=error_om_p->level;
    error_info_p->md_name_len = sharmem_data_p->md_ag[error_om_p->md_ar_idx].name_length;
    error_info_p->ma_name_len = sharmem_data_p->ma_ag[error_om_p->ma_ar_idx].name_length;
    error_info_p->mep_id=error_om_p->mp_id;
    error_info_p->vlan_id=error_om_p->vid;
    error_info_p->reason=error_om_p->last_reason;
    error_info_p->reason_bit_map=error_om_p->reason_bit_map;
    error_info_p->lport         = error_om_p->lport;
    return TRUE;
}/*End of CFM_OM_LocalFillErrorInfo*/
#if 0
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------------
 * FUNCTION : CFM_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for CFM OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void CFM_OM_AttachSystemResources(void)
{
    sharmem_data_p = (CFM_OM_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_CFM_OM_SHMEM_SEGID);

    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_OM, &cfm_om_sem_id);
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the cfm size for share memory
 * INPUT    : *segid_p
 *                 *seglen_p
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_OM_GetShMemInfo(
                        SYSRSC_MGR_SEGID_T *segid_p,
                        UI32_T *seglen_p)
{
    L_HISAM_ShMem_Desc_T   mep_hisam_des;
    L_HISAM_ShMem_Desc_T   mip_hisam_des;
    L_HISAM_ShMem_Desc_T   remote_mep_hisam_des;
    L_HISAM_ShMem_Desc_T   ltr_hisam_des;
    *seglen_p=0;
    mep_hisam_des.hash_depth         = HASH_DEPTH;
    mep_hisam_des.N1                 = HISAM_N1;
    mep_hisam_des.N2                 = HISAM_N2;
    mep_hisam_des.record_length      = MEP_NODE_LEN;
    mep_hisam_des.total_hash_nbr     = MEP_HASH_NBR;
    mep_hisam_des.total_index_nbr    = INDEX_NBR;
    mep_hisam_des.total_record_nbr   = MEP_NODE_NBR;
    *seglen_p+=L_HISAM_ShMem_GetWorkingBufferRequiredSize(&mep_hisam_des, MEP_NUMBER_OF_KEYS);

    mip_hisam_des.hash_depth         = HASH_DEPTH;
    mip_hisam_des.N1                 = HISAM_N1;
    mip_hisam_des.N2                 = HISAM_N2;
    mip_hisam_des.record_length      = MIP_NODE_LEN;
    mip_hisam_des.total_hash_nbr     = MIP_HASH_NBR;
    mip_hisam_des.total_index_nbr    = INDEX_NBR;
    mip_hisam_des.total_record_nbr   = MIP_NODE_NBR;
    *seglen_p+=L_HISAM_ShMem_GetWorkingBufferRequiredSize(&mip_hisam_des, MIP_NUMBER_OF_KEYS);

    remote_mep_hisam_des.hash_depth         = HASH_DEPTH;
    remote_mep_hisam_des.N1                 = HISAM_N1;
    remote_mep_hisam_des.N2                 = HISAM_N2;
    remote_mep_hisam_des.record_length      = REMOTE_MEP_NODE_LEN;
    remote_mep_hisam_des.total_hash_nbr     = REMOTE_MEP_HASH_NBR;
    remote_mep_hisam_des.total_index_nbr    = INDEX_NBR;
    remote_mep_hisam_des.total_record_nbr   = REMOTE_MEP_NODE_NBR;
    *seglen_p+=L_HISAM_ShMem_GetWorkingBufferRequiredSize(&remote_mep_hisam_des, REMOTE_MEP_NUMBER_OF_KEYS);

    ltr_hisam_des.hash_depth         = HASH_DEPTH;
    ltr_hisam_des.N1                 = HISAM_N1;
    ltr_hisam_des.N2                 = HISAM_N2;
    ltr_hisam_des.record_length      = LTR_NODE_LEN;
    ltr_hisam_des.total_hash_nbr     = LTR_HASH_NBR;
    ltr_hisam_des.total_index_nbr    = INDEX_NBR;
    ltr_hisam_des.total_record_nbr   = LTR_NODE_NBR;

    *seglen_p+=L_HISAM_ShMem_GetWorkingBufferRequiredSize(&ltr_hisam_des, LTR_NUMBER_OF_KEYS);


    *segid_p = SYSRSC_MGR_CFM_OM_SHMEM_SEGID;
    *seglen_p += sizeof(CFM_OM_ShmemData_T);
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_Init(void)
{
    flag=0;
    CFM_TIMER_Init();
    /*create mep, remote mep and ltr hisam table*/
    CFM_OM_LocalCreateHisamTable();

    /*memset  the global variable*/
    memset(&sharmem_data_p->global_cfg_g, 0, sizeof(CFM_OM_GlobalConfig_T));
    /*init the ltr queue list*/
    memset(&sharmem_data_p->ltr_queue_header_g, 0,sizeof(CFM_OM_LTR_QUEUE_HEAD_T));
    sharmem_data_p->ltr_queue_header_g.first_pdu_p=NULL;
    sharmem_data_p->ltr_queue_header_g.last_pdu_p=NULL;
    /*init error list*/
    memset(&sharmem_data_p->error_list_ag, 0,sizeof(CFM_OM_ErrorListHead_T));
    sharmem_data_p->error_list_head_g.first_ar_idx=-1;
    sharmem_data_p->error_list_head_g.last_ar_idx=-1;
    /*init loopbck list*/
    memset(&sharmem_data_p->loop_back_list_head_g, 0, sizeof(CFM_OM_LoopBackListHead_T));
    sharmem_data_p->loop_back_list_head_g.first_ar_idx=-1;
    sharmem_data_p->loop_back_list_head_g.last_ar_idx=-1;
    /*init port cfm status*/
    memset(sharmem_data_p->cfm_port_status_ag, 0, sizeof(sharmem_data_p->cfm_port_status_ag));

    {
        UI16_T i;
        memset(sharmem_data_p->md_ag, 0, sizeof(sharmem_data_p->md_ag));
        memset(sharmem_data_p->ma_ag, 0, sizeof(sharmem_data_p->ma_ag));
        sharmem_data_p->md_head_index=-1;

        for(i=0;i<SYS_ADPT_CFM_MAX_NBR_OF_MD;i++)
        {
            sharmem_data_p->md_ag[i].next_ar_idx=-1;
            sharmem_data_p->ma_ag[i].next_ar_idx=-1;
        }
    }
    return;
}/*end of CFM_OM_Init*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_EnterSlaveMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM value
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(sharmem_data_p);
    return;
}/*end of CFM_OM_EnterSlaveMode*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_EnterTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM value
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_EnterTransitionMode()
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(sharmem_data_p);

    /*claer the timer list*/
    CFM_TIMER_DestroyAllTimer();
    /*clear loop back list*/
    CFM_OM_ClearLoopBackList();
    /*clear the link error list*/
    CFM_OM_ClearErrorsList();
    /*clear the ltr pending list*/
    CFM_OM_DeleteAllLTrQueueElement();
    /*free all mep record*/
    L_HISAM_ShMem_DeleteAllRecord(&sharmem_data_p->mep_hisam_des_g);

    /*clear the mip*/
    L_HISAM_ShMem_DeleteAllRecord(&sharmem_data_p->mip_hisam_des_g);

    L_HISAM_ShMem_DeleteAllRecord(&sharmem_data_p->remote_mep_hisam_des_g);

    L_HISAM_ShMem_DeleteAllRecord(&sharmem_data_p->ltr_hisam_des_g);

    {
        UI16_T i;

        memset(sharmem_data_p->md_ag, 0, sizeof(sharmem_data_p->md_ag));
        memset(sharmem_data_p->ma_ag, 0, sizeof(sharmem_data_p->ma_ag));
        sharmem_data_p->md_head_index=-1;

        for(i=0;i<SYS_ADPT_CFM_MAX_NBR_OF_MD;i++)
        {
            sharmem_data_p->md_ag[i].next_ar_idx=-1;
            sharmem_data_p->ma_ag[i].next_ar_idx=-1;
        }
    }
    return;
}/*end of CFM_OM_EnterTransitionMode*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetOperatingMode
 * ------------------------------------------------------------------------
 * PURPOSE  : get the operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T CFM_OM_GetOperatingMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sharmem_data_p);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE  : set the operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(sharmem_data_p);
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_EnterMasterMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM and global value
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void CFM_OM_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(sharmem_data_p);
    {/*set the globl varaible to default value*/

        sharmem_data_p->global_cfg_g.remote_mep_start_delay=SYS_DFLT_CFM_CROSSCHECK_START_DELAY;
        sharmem_data_p->global_cfg_g.cfm_status =SYS_DFLT_CFM_GLOBAL_STATUS;
        sharmem_data_p->global_cfg_g.link_trace_cache_status=SYS_DFLT_CFM_LINKTRACE_CACHE_STATUS;
        sharmem_data_p->global_cfg_g.link_trace_hold_time=SYS_DFLT_CFM_LINKTRACE_HOLD_TIME*60;
        /*set the link trace entries*/
        sharmem_data_p->global_cfg_g.link_trace_size=SYS_DFLT_CFM_LINKTRACE_CACHE_SIZE;
        sharmem_data_p->link_trace_entries_g=0;
        sharmem_data_p->md_next_index_g=1;
    }

    {/*set each port to default value*/
        UI32_T ifindex;

        for(ifindex=1; ifindex<=SYS_ADPT_TOTAL_NBR_OF_LPORT;ifindex++)
        {
            CFM_OM_SetCFMPortStatus(ifindex, SYS_DFLT_CFM_PORT_STATUS);
        }
    }
    /*set snmp trap to default*/
    sharmem_data_p->global_cfg_g.snmp_cc_mep_up_trap=SYS_DFLT_CFM_SNMP_TRAP_CC_MEP_UP;
    sharmem_data_p->global_cfg_g.snmp_cc_mep_down_trap=SYS_DFLT_CFM_SNMP_TRAP_CC_MEP_DOWN;
    sharmem_data_p->global_cfg_g.snmp_cc_config_trap=SYS_DFLT_CFM_SNMP_TRAP_CC_CONFIG;
    sharmem_data_p->global_cfg_g.snmp_cc_loop_trap=SYS_DFLT_CFM_SNMP_TRAP_CC_LOOP;
    sharmem_data_p->global_cfg_g.snmp_cross_check_mep_unknown_trap=SYS_DFLT_CFM_SNMP_TRAP_CROSS_CHECK_UNKNOWN;
    sharmem_data_p->global_cfg_g.snmp_cross_check_mep_missing_trap=SYS_DFLT_CFM_SNMP_TRAP_CROSS_CHECK_MEP_MISSING;
    sharmem_data_p->global_cfg_g.snmp_cross_check_ma_up_trap=SYS_DFLT_CFM_SNMP_TRAP_CROSS_CHECK_MA_UP;

#if (SYS_CPNT_CFM_DELAY_MEASUREMENT == TRUE)
    memset(&sharmem_data_p->dmm_ctr_rec_g, 0, sizeof(sharmem_data_p->dmm_ctr_rec_g));
#endif /* #if (SYS_CPNT_CFM_DELAY_MEASUREMENT == TRUE) */

    memset(&sharmem_data_p->lbm_ctr_rec_g, 0, sizeof(sharmem_data_p->lbm_ctr_rec_g ));
}/*end of CFM_OM_EnterMasterMode*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get the mep pointer from the hisame table
 * INPUT    : md_index   - the maintenance domain index
 *            ma_index   - the maintenance association index
 *            l_port     - the logical port
 *            key_type   - use which key type
 *                       - CFM_OM_MD_MA_MEP_KEY
 *                       - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT   : mep_p       - the pointer of mep store in hisam
 * RETUEN   : TRUE       - get success
 *            FALSE      - get fail, no record.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMep(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T mep_id,
                    UI32_T l_port,
                    UI16_T key_type,
                    CFM_OM_MEP_T *mep_p)
{
    memset(mep_p, 0, sizeof (CFM_OM_MEP_T));

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;
    mep_p->lport=l_port;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMep
 * ------------------------------------------------------------------------
 * PURPOSE  :This will get the next  mep pointer from the hisame table
 * INPUT    :md_index  - the maintenance domain index
 *           ma_index  - the maintenance association index
 *           l_port    - the logical port
 *           key_type  - use which key type
 *                     - CFM_OM_MD_MA_MEP_KEY
 *                     - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT   :mep_p      - the pointer of mep store in hisam
 * RETUEN   :TRUE      - get success
 *           FALSE     - get fail, no record.
 * NOTES    :None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMep(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T l_port ,
                        UI16_T key_type,
                        CFM_OM_MEP_T *mep_p)
{
    memset(mep_p, 0, sizeof (CFM_OM_MEP_T));

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;
    mep_p->lport=l_port;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetNextMepRecord(mep_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetNextMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new  mep pointer to the hisame table
 * INPUT    : *md_p     - the maintenance domain pointer
 *            *ma_p     - the maintenance association pointer
 *            l_port    - the logical port
 *            direction - the mep direction
 * OUTPUT   :
 * RETUEN   : TRUE      - add success
 *            FALSE     - add fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewMep(
                        CFM_OM_MD_T *md_p,
                        CFM_OM_MA_T *ma_p,
                        UI32_T mep_id,
                        UI32_T l_port,
                        CFM_TYPE_MP_Direction_T direction)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    CFM_OM_LocalInitMep(md_p, ma_p, mep_p, mep_id, l_port, direction);

    mep_p->md_index=md_p->index;
    mep_p->ma_index=ma_p->index;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_AddNewMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a delete a  mep pointer from the hisame table
 * INPUT    : md_index - the maintenance domain index
 *            ma_index - the maintenance association index
 *            mep_id   - the mep id
 *            l_port   - the logical port
 *            key_type - use which key type
 *                     - CFM_OM_MD_MA_MEP_KEY
 *                     - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT   :
 * RETUEN   : TRUE     - delet success
 *            FALSE    - delet fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMep(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T l_port,
                        UI16_T key_type)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;
    mep_p->lport=l_port;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, key_type))
    {
        CFM_OM_UNLOCK();
        /*this record donesn't exist*/
        return TRUE;
    }

    /*delete record from hisam need use the primary key, it means first key in key define table
      so assign the mep id for key_type parameter is use second key
     */
    if (TRUE == CFM_OM_LocalDeleteMepRecord(mep_p, CFM_OM_LPORT_MD_MA_KEY))
    {
        CFM_OM_UNLOCK();
        return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*end of CFM_OM_DeleteMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMep
 * ------------------------------------------------------------------------
 * PURPOSE : This function will modify the mep key in hisam table
 * INPUT   : md_index   - the maintenance domain index
 *           ma_index   - the maintenance association index
 *           old_mep_id - the original mep id
 *           old_lport  - the origianl logical port
 *           new_mep_id - the new mep id
 *           new_lport  - the new lport
 *           key_type   - use which key type
 *                      - CFM_OM_MD_MA_MEP_KEY
 *                      - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT  :
 * RETUEN  : TRUE       - success
 *           FALSE      - fail
 * NOTES   : because key change, so must delete the old record then add new record
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMep(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T old_mep_id,
                    UI32_T old_lport,
                    UI32_T new_mep_id,
                    UI32_T new_lport,
                    UI16_T key_type)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=old_mep_id;
    mep_p->lport=old_lport;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    if (FALSE == CFM_OM_LocalDeleteMepRecord(mep_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->identifier=new_mep_id;
    mep_p->lport=new_lport;

    if (FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_StoreMep
 * ------------------------------------------------------------------------
 * PURPOSE : This function store the mep's all information again
 * INPUT   : mep_p - the pointer to mep which has content to store
 * OUTPUT  :
 * RETUEN  : TRUE       - success
 *           FALSE      - fail
 * NOTES   : because key change, so must delete the old record then add new record
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_StoreMep(
                       CFM_OM_MEP_T *mep_p )
{
    if(NULL == mep_p)
        return FALSE;
    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetMep*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMeplport
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's logical port
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lport     - the logical port
 *            new_mac_p - pointer to new mac address
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.delete the old mep record from hisam table
 *           2. add new mep record to hisam table
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMeplport(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T  lport,
                        UI8_T   *new_mac_p)
{
    CFM_OM_MEP_T    *mep_p=&mep_tmp3_g;
    BOOL_T          ret = FALSE;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;
    mep_p->lport=lport;

    CFM_OM_LOCK();
    /*check the new lport already has mep_p exist or not. if has return fail, because duplicate*/
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_LPORT_MD_MA_KEY))
    {
        if (TRUE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
        {
            if(TRUE == CFM_OM_LocalDeleteMepRecord(mep_p, CFM_OM_LPORT_MD_MA_KEY))
            {
                mep_p->lport=lport;
                memcpy(mep_p->mac_addr_a, new_mac_p, SYS_ADPT_MAC_ADDR_LEN);
                ret = CFM_OM_LocalSetMepRecord(mep_p);
            }
        }
    }
    CFM_OM_UNLOCK();
    return ret;
}/*End of CFM_OM_SetMeplport*/

#if 0 /* done by CFM_ENGINE_SetMepDirection */
/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepDirection
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's logical port
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lport   - the logical port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepDirection(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            CFM_TYPE_MP_Direction_T direction)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->direction=direction;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepDirection*/
#endif /* #if 0 */

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepPrimaryVid
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's primary vid
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            lport   - the logical port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepPrimaryVid(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI16_T primary_vid)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->primary_vid=primary_vid;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepPrimaryVid*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepActive
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's active status
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            active_status- the mep active status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepActive(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        BOOL_T active_status)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->active=active_status;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepActive*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepCciStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's dirction
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            status    - the cci status
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :  CFM_TYPE_CCI_STATUS_ENABLE
 *             CFM_TYPE_CCI_STATUS_DISABLE
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepCciStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            CFM_TYPE_CcmStatus_T cci_status)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->cci_status=cci_status;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepCciStatus*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepCcmLtmPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ccm_ltm_priority-the ccm and ltm packet priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepCcmLtmPriority(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T ccm_ltm_priority)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->ccm_ltm_priority=ccm_ltm_priority;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepCcmLtmPriority*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLowPrDef
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lowest alarm priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            low_pri   - the lowerst defect priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :  CFM_TYPE_FNG_LOWEST_ALARM_ALL
 *             CFM_TYPE_FNG_LOWEST_ALARM_MAC_REM_ERR_XCON
 *             CFM_TYPE_FNG_LOWEST_ALARM_REM_ERR_XCON
 *             CFM_TYPE_FNG_LOWEST_ALARM_ERR_XCON
 *             CFM_TYPE_FNG_LOWEST_ALARM_XCON
 *             CFM_TYPE_FNG_LOWEST_ALARM_NO_DEF
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLowPrDef(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            CFM_TYPE_FNG_LowestAlarmPri_T low_pri)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->lowest_priority_defect=low_pri;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepLowPrDef*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepFngAlarmTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fng alarm time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            alarm_time  - the fault alarm time by ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepFngAlarmTime(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T alarm_time)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->fng_alarm_time=alarm_time;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepFngResetTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's fault alarm reset time
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            reset_time- the fault alarm reset time by ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepFngResetTime(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T reset_time)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->fng_reset_time=reset_time;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepFngResetTime*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmDstMac
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mac   - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmDstMac(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    memcpy(mep_p->transmit_lbm_dest_mac_address_a, dst_mac, SYS_ADPT_MAC_ADDR_LEN);

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepLbmDstMac*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmDestMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's destination mac address
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mep_id - the lbm destination mac
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function only for MIB
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmDestMepId(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T dst_mep_id)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->transmit_lbm_dest_mep_id=dst_mep_id;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepLbmDestMepId*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmTargetIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's lbm destination mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            dst_mep_id- the lbm destination mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmTargetIsMepId(
                                    UI32_T md_index,
                                    UI32_T ma_index,
                                    UI32_T mep_id,
                                    UI8_T dst_mep_id)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->transmit_lbm_dest_is_mep_id=dst_mep_id;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepLbmTargetIsMepId*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmMessages
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit the lbm counts
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            counts    - the lbm message counts
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmMessages(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI32_T counts)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->transmit_lbm_messages=counts;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*CFM_OM_SetMepLbmMessages*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLbmVlanPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's transmit blm vlan priority
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            priority  - the lbm priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLbmVlanPriority(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T priority)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->transmit_lbm_vlan_priority=priority;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmFlags
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm transmit flags
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 *            is_useFDBonly - set only use the fdb or not
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmFlags(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            BOOL_T is_useFDBonly)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->ltm_use_fdb_only=is_useFDBonly;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmTargetMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mac
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mac-the ltm target address
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmTargetMacAddress(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T target_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    memcpy(mep_p->transmit_ltm_target_mac_address_a,target_mac, SYS_ADPT_MAC_ADDR_LEN);

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmTargetMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's target mp id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            target_mep_id - the target mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmTargetMepId(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T target_mep_id)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->transmit_ltm_target_mep_id=target_mep_id;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepLtmTargetMepId*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmTargetIsMepId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm target is the mep id
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            is_mep_id - the ltm target is the mep id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmTargetIsMepId(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                BOOL_T is_mep_id)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->transmit_ltm_target_is_mep_id=is_mep_id;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepLtmTargetIsMepId*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMepLtmTtl
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set mep's ltm ttl
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            mep_id    - the mep id
 *            ttl       - the trnamsmit ttl
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMepLtmTtl(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T ttl)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->transmit_ltm_ttl=ttl;

    if(FALSE == CFM_OM_LocalSetMepRecord(mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMepLtmTtl*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMip
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get the mip pointer from the hisame table
 * INPUT    : md_index   - the maintenance domain index
 *            ma_index   - the maintenance association index
 *            lport      - the logical port
 *            key_type   - use which key type
 *                       - CFM_OM_LPORT_MD_MA_KEY
 *                        CFM_OM_MD_MA_LPORT_KEY
 * OUTPUT   : *mip_pp   - the pointer of mip store in hisam
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMip(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T lport,
                    UI16_T key_type,
                    CFM_OM_MIP_T *mip_p)
{
    memset(mip_p, 0, sizeof (CFM_OM_MIP_T));

    mip_p->md_index=md_index;
    mip_p->ma_index=ma_index;
    mip_p->lport=lport;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMipExactRecord(mip_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetMip*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMip
 * ------------------------------------------------------------------------
 * PURPOSE  :This will get the next  mip pointer from the hisame table
 * INPUT    :md_index   - the maintenance domain index
 *           ma_index   - the maintenance association index
 *           key_type   - use which key type
 *           lport      - the logical port
 *                      - CFM_OM_LPORT_MD_MA_KEY
 *                        CFM_OM_MD_MA_LPORT_KEY
 * OUTPUT   :*mip_pp   - the pointer of mip store in hisam
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    :None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMip(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T lport,
                        UI16_T key_type,
                        CFM_OM_MIP_T *mip_p)
{
    memset(mip_p, 0, sizeof (CFM_OM_MIP_T));

    mip_p->md_index=md_index;
    mip_p->ma_index=ma_index;
    mip_p->lport=lport;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetNextMipRecord(mip_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetNextMip*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewMip
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new mip pointer to the hisame table
 * INPUT    : md_p   - the maintenance domain pointer
 *            ma_p   - the maintenance association pointer
 *            lport  - the logical port, the mip create on
 *            mac  - the mip mac
 * OUTPUT   :
 * RETUEN   : TRUE   - success
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewMip(
                        CFM_OM_MD_T *md_p,
                        CFM_OM_MA_T *ma_p,
                        UI32_T lport,
                        UI8_T mac_a[SYS_ADPT_MAC_ADDR_LEN])
{
    CFM_OM_MIP_T mip;

    CFM_OM_LocalInitMip(&mip, md_p, ma_p,lport, mac_a);

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalSetMipRecord(&mip))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_AddNewMip*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMip
 * ------------------------------------------------------------------------
 * PURPOSE : This function will modify the mip key in hisam table
 * INPUT   : md_index   - the maintenance domain index
 *           ma_index   - the maintenance association index
 *           old_lport  - the origianl logical port
 *           new_lport  - the new lport
 *           key_type   - use which key type
 *                      - CFM_OM_MD_MA_LPORT_KEY
 *                      - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT  :
 * RETUEN  : TRUE       - success
 *           FALSE      - fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMip(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T old_lport,
                    UI32_T new_lport,
                    UI16_T key_type)
{
    CFM_OM_MIP_T mip;

    mip.md_index=md_index;
    mip.ma_index=ma_index;
    mip.lport=old_lport;

    CFM_OM_LOCK();
    /*because lport is key, so it need delete then add new one*/
    if (FALSE == CFM_OM_LocalGetMipExactRecord(&mip, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    if(FALSE == CFM_OM_LocalDeleteMipRecord(&mip, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mip.lport=new_lport;

    if(FALSE == CFM_OM_LocalSetMipRecord(&mip))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetMip*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_StoreMip
 * ------------------------------------------------------------------------
 * PURPOSE : This function store the mip's all information again
 * INPUT   : mip_p - the pointer to mep which has content to store
 * OUTPUT  :
 * RETUEN  : TRUE       - success
 *           FALSE      - fail
 * NOTES   : because key change, so must delete the old record then add new record
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_StoreMip(
                       CFM_OM_MIP_T *mip_p )
{
    if(NULL == mip_p)
        return FALSE;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalSetMipRecord(mip_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_StoreMip*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMip
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a delete a  mip pointer from the hisame table
 * INPUT    : md_index- the maintenance domain index
 *            ma_index- the maintenance association index
 *            lport   - the logical port
 *            key_type - CFM_OM_LPORT_MD_MA_KEY
 * OUTPUT   :
 * RETUEN   : TRUE    - success
 *            FALSE   - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMip(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T lport,
                        UI16_T key_type)
{
    CFM_OM_MIP_T mip;

    mip.md_index=md_index;
    mip.ma_index=ma_index;
    mip.lport=lport;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetMipExactRecord(&mip, key_type))
    {
        CFM_OM_UNLOCK();
        /*this record donesn't exist*/
        return TRUE;
    }

    if (TRUE == CFM_OM_LocalDeleteMipRecord(&mip, key_type))
    {
        CFM_OM_UNLOCK();
        return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*end of CFM_OM_DeleteMip*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRemoteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get a remote mep pointer from the hisame table
 * INPUT    : md_index  - the maintenance domain index of rcvd md
 *            ma_index  - the maintenance association index of rcvd ma
 *            mep_id    - the mep id of remote mep
 *            key_type  - use which key type
 *                       - CFM_OM_MD_MA_MEP_KEY
 * OUTPUT   : *remote_mep_pp - the pointer of remote mep store in hisam
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetRemoteMep(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T key_type,
                        CFM_OM_REMOTE_MEP_T *remote_mep_p)
{
    memset(remote_mep_p, 0, sizeof (CFM_OM_REMOTE_MEP_T));

    remote_mep_p->md_index=md_index;
    remote_mep_p->ma_index=ma_index;
    remote_mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE== CFM_OM_LocalGetRemoteMepExactRecord(remote_mep_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetRemoteMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRemoteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get next remote mep pointer from the hisame table
 * INPUT    : md_index   - the maintenance domain index of rcvd md
 *            ma_index   - the maintenance association index of rcvd ma
 *            mep_id     - the mep id or remote
 *            key_type   - use which key type
 *                        - CFM_OM_MD_MA_MEP_KEY
 * OUTPUT   : remote_mep_p - the pointer of mep store in hisam
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextRemoteMep(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI16_T key_type,
                            CFM_OM_REMOTE_MEP_T *remote_mep_p)
{
    memset(remote_mep_p, 0, sizeof (CFM_OM_REMOTE_MEP_T));

    remote_mep_p->md_index=md_index;
    remote_mep_p->ma_index=ma_index;
    remote_mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetNextRemoteMepRecord(remote_mep_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetNextRemoteMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewRemoteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new remote mep pointer to the hisame table
 * INPUT    : *md_p    - the maintenance domain pointer of the rcvd mep
 *            *ma_p    - the maintenance association pointer of the rcvd mep
 *            r_mep_id   - the remote mep pointer want to save in hisam
 * OUTPUT   :
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewRemoteMep(
                            CFM_OM_MD_T *md_p,
                            CFM_OM_MA_T *ma_p,
                            UI32_T r_mep_id)
{
    CFM_OM_REMOTE_MEP_T r_mep;

    CFM_OM_LocalInitRemoteMep(&r_mep, r_mep_id, md_p, ma_p);

    CFM_OM_LOCK();
    if (TRUE == CFM_OM_LocalSetRemoteMepRecord(&r_mep))
    {
        CFM_OM_UNLOCK();
        return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*end of CFM_OM_AddNewRemoteMep*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_ResetRemoteMepData
 *-------------------------------------------------------------------------
 * PURPOSE  : This function initial the remote mep
 * INPUT    : *r_mep_p   - the remote mep pointer
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void CFM_OM_ResetRemoteMepData(
                        CFM_OM_REMOTE_MEP_T *r_mep_p)
{
    r_mep_p->interface_status=CFM_TYPE_INTERFACE_STATUS_NO_INTERFACE_STATUS_TLV;
    r_mep_p->port_status=CFM_TYPE_PORT_STATUS_NO_PORT_STATE_TLV;

    r_mep_p->mac_addr_a[0]=0xff;
    r_mep_p->mac_addr_a[1]=0xff;
    r_mep_p->mac_addr_a[2]=0xff;
    r_mep_p->mac_addr_a[3]=0xff;
    r_mep_p->mac_addr_a[4]=0xff;
    r_mep_p->mac_addr_a[5]=0xff;

    r_mep_p->cc_life_time=0;
    r_mep_p->frame_loss=0;
    r_mep_p->packet_received=0;
    r_mep_p->packet_error=0;

    r_mep_p->mep_up=FALSE;

    CFM_OM_LOCK();
    CFM_OM_LocalSetRemoteMepRecord(r_mep_p);
    CFM_OM_UNLOCK();
}/* End of CFM_OM_ResetRemoteMepData*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteRemoteMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This will delete a remote mep pointer from the hisame table
 * INPUT    : md_index   - the maintenance domain index of the rcvd mep
 *            ma_index   - the maintenance association index of the rcvd mep
 *            r_mep_id     - the mep id of the remote mep
 *            key_type   - use which key type
 *                         - CFM_OM_MD_MA_MEP_KEY
 * OUTPUT   :
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteRemoteMep(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T r_mep_id,
                            UI32_T key_type)
{
    CFM_OM_REMOTE_MEP_T r_mep;

    r_mep.md_index=md_index;
    r_mep.ma_index=ma_index;
    r_mep.identifier=r_mep_id;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetRemoteMepExactRecord(&r_mep, key_type))
    {
        CFM_OM_UNLOCK();
        /*this record donesn't exist*/
        return TRUE;
    }

    if (TRUE == CFM_OM_LocalDeleteRemoteMepRecord(&r_mep, key_type))
    {
        CFM_OM_UNLOCK();
        return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*end of CFM_OM_DeleteRemoteMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMep
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will modify the mep content in hisam table
 * INPUT    :  r_mep_p       - the content will replace the old content in hiame table
 * OUTPUT   :
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetRemoteMep(
                    CFM_OM_REMOTE_MEP_T *r_mep_p)
{
    if(NULL == r_mep_p)
        return FALSE;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalSetRemoteMepRecord(r_mep_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetRemoteMep*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new  mep pointer to the hisame table
 * INPUT    : md_index  - the maintenance domain index
 *            ma_index  - the maintenance association index
 *            mep_id    - the mep id
 *            seq_num   - the seq_num of ltr
 *            rcvd_order- the receive ltr order
 *            key_type  - use which key type
 *                       - CFM_OM_MD_MA_MEP_SEQ_REC_KEY
 * OUTPUT   : ltr_p      - the pointer of ltr store in hisam
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetLtr(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T mep_id,
                    UI32_T seq_num,
                    UI32_T rcvd_order,
                    UI32_T key_type,
                    CFM_OM_LTR_T *ltr_p)
{
    memset(ltr_p, 0, sizeof (CFM_OM_LTR_T));

    ltr_p->md_index=md_index;
    ltr_p->ma_index=ma_index;
    ltr_p->rcvd_mep_id=mep_id;
    ltr_p->receive_order=rcvd_order;
    ltr_p->seq_number=seq_num;
    CFM_OM_LOCK();
    if (TRUE == CFM_OM_LocalGetLtrExactRecord(ltr_p, key_type))
    {
        CFM_OM_UNLOCK();
        return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*end of CFM_OM_GetLtr*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will get next ltr pointer from the hisame table
 * INPUT    : md_index  - the maintenance domain index
 *            ma_index  - the maintenance association index
 *            mep_id    - the mep id
 *            seq_num   - the seq_num of ltr
 *            rcvd_order- the receive ltr order
 *            key_type  - use which key type
 *                       - CFM_OM_MD_MA_MEP_SEQ_REC_KEY
 * OUTPUT   : pLtr       - the pointer of ltr store in hisam
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextLtr(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T seq_num,
                        UI32_T rcvd_order,
                        UI32_T key_type,
                        CFM_OM_LTR_T *ltr_p)
{
    memset(ltr_p, 0, sizeof (CFM_OM_LTR_T));

    ltr_p->md_index=md_index;
    ltr_p->ma_index=ma_index;
    ltr_p->rcvd_mep_id=mep_id;
    ltr_p->receive_order=rcvd_order;
    ltr_p->seq_number=seq_num;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetNextLtrRecord(ltr_p, key_type))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetNextLtr */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_IsLTCacheFull
 * ------------------------------------------------------------------------
 * PURPOSE  : To chcek if link trace cache is full
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE  - FULL
 *            FALSE - NOT FULL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_IsLTCacheFull(void)
{
    BOOL_T  ret = FALSE;

    CFM_OM_LOCK();
    if(sharmem_data_p->link_trace_entries_g >= sharmem_data_p->global_cfg_g.link_trace_size)
    {
        ret = TRUE;
    }
    CFM_OM_UNLOCK();
    return ret;
}/*end of CFM_OM_IsLTCacheFull */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a new ltr pointer to the hisame table
 * INPUT    : ltr_p - the pointer to the content of ltr
 * OUTPUT   : None
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddLtr(
                    CFM_OM_LTR_T *ltr_p)
{
    if(sharmem_data_p->link_trace_entries_g>=sharmem_data_p->global_cfg_g.link_trace_size)
    {
        return FALSE;
    }
    CFM_OM_LOCK();
    sharmem_data_p->link_trace_entries_g++;

    if (TRUE == CFM_OM_LocalSetLtrRecord(ltr_p))
    {
        CFM_OM_UNLOCK();
        return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*end of CFM_OM_AddLtr*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will add a delete ltr pointer from the hisame table
 * INPUT    : ltr_p - the ltr which want to delete
 * OUTPUT   :
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : The key value already exsit in ltr
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteLtr(
                 CFM_OM_LTR_T *ltr_p)
{
    BOOL_T  ret = FALSE;

    CFM_OM_LOCK();
    ret = CFM_OM_LocalDeleteLtrRecord(ltr_p, CFM_OM_MD_MA_MEP_SEQ_REC_KEY);
    if (TRUE == ret)
    {
        sharmem_data_p->link_trace_entries_g --;
    }
    CFM_OM_UNLOCK();
    return ret;
}/*end of CFM_OM_DeleteLtr*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetLtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This will modify the ltr contnet in hisam table
 * INPUT    :  ltr_p       - the point to the content which will replace the old ltr in hisam table
 * OUTPUT   :
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetLtr(
                        CFM_OM_LTR_T *ltr_p)
{
    if(NULL == ltr_p)
        return FALSE;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalSetLtrRecord(ltr_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetLtr*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByMdMaIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the input level and vid.
 * INPUT    : ma_ar_idx        - ma index
 * OUTPUT   : **return_md_p - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByMdMaIndex(
                                UI32_T md_index,
                                UI32_T ma_indx,
                                CFM_OM_MD_T **return_md_pp,
                                CFM_OM_MA_T **return_ma_pp)
{
    BOOL_T ret=FALSE;
    CFM_OM_LOCK();
    ret=CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_indx, return_md_pp, return_ma_pp);
    CFM_OM_UNLOCK();
    return ret;
}/*end of CFM_OM_GetMdMaByMdMaIndex*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the input ma name.
 * INPUT    : *ma_name       - the ma name pointer
 *            name_len       - ma name length
 * OUTPUT   : **return_md_pp - the md record
 *            **return_ma_pp - the ma record
 * RETUEN   : TRUE           - success
 *            FALSE          - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByMaName(
                            UI8_T *ma_name_ap,
                            UI32_T name_len,
                            CFM_OM_MD_T **return_md_pp,
                            CFM_OM_MA_T **return_ma_pp)
{
    BOOL_T ret=FALSE;
    CFM_OM_LOCK();
    ret=CFM_OM_LocalGetMdMaByMaName(ma_name_ap, name_len, return_md_pp, return_ma_pp);
    CFM_OM_UNLOCK();
    return ret;
}/*end of CFM_OM_GetMdMaByMaName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByMdMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : To get pointer to md/ma for specified md and ma name.
 * INPUT    : md_name_p    - the md name pointer
 *            md_name_len  - md name length
 *            ma_name_p    - the ma name pointer
 *            ma_name_len  - ma name length
 * OUTPUT   : return_md_pp - the md record
 *            return_ma_pp - the ma record
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByMdMaName(
    UI8_T           *md_name_p,
    UI32_T          md_name_len,
    UI8_T           *ma_name_p,
    UI32_T          ma_name_len,
    CFM_OM_MD_T     **return_md_pp,
    CFM_OM_MA_T     **return_ma_pp)
{
    BOOL_T ret=FALSE;
    CFM_OM_LOCK();
    ret = CFM_OM_LocalGetMdMaByMdMaName(
                    md_name_p, md_name_len, ma_name_p, ma_name_len, return_md_pp, return_ma_pp);
    CFM_OM_UNLOCK();
    return ret;
}/*end of CFM_OM_GetMdMaByMaName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByMdIndxMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the input ma name.
 * INPUT    : md_index       - the md index
 *            *ma_name_ap    - the ma name array pointer
 *            name_len       - the ma name length
 * OUTPUT   : **return_md_pp - the md record
 *            **return_ma_pp - the ma record
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByMdIndxMaName(
                                UI32_T md_index,
                                UI8_T *ma_name_ap,
                                UI32_T name_len,
                                CFM_OM_MD_T **return_md_pp,
                                CFM_OM_MA_T **return_ma_pp)
{
    BOOL_T ret=FALSE;
    CFM_OM_LOCK();
    ret=CFM_OM_LocalGetMdMaByMdIndxMaName(md_index, ma_name_ap, name_len, return_md_pp, return_ma_pp);
    CFM_OM_UNLOCK();
    return ret;
}/*end of CFM_OM_GetMdMaByMdIndxMaName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaByLevelVid
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md and ma index according to the level vid
 * INPUT    : level         - the md level
 *            vid           - the vlan id
 * OUTPUT   : *return_md_p  - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaByLevelVid(
                            CFM_TYPE_MdLevel_T level,
                            UI16_T vid,
                            CFM_OM_MD_T **return_md_pp,
                            CFM_OM_MA_T **return_ma_pp)
{
    BOOL_T ret=FALSE;
    CFM_OM_LOCK();
    ret=CFM_OM_LocalGetMdMaByLevelVid(level, vid, return_md_pp, return_ma_pp);
    CFM_OM_UNLOCK();
    return ret;
}/*end of CFM_OM_GetMdMaByLevelVid*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFirstHighLevelMdMaBylevelVid
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the first low level md and ma index according to the level vid
 * INPUT    : level         - the md level
 *            vid           - the vlan id
 * OUTPUT   : *return_md_p  - the md record
 *            **return_ma_p - the ma record
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetFirstHighLevelMdMaBylevelVid(
                                CFM_TYPE_MdLevel_T level,
                                UI16_T vid,
                                CFM_OM_MD_T **return_md_pp,
                                CFM_OM_MA_T **return_ma_pp)
{
    BOOL_T  ret=FALSE;
    CFM_OM_LOCK();
    ret=CFM_OM_LocalGetFirstHighLevelMdMaBylevelVid(level, vid, return_md_pp, return_ma_pp);
    CFM_OM_UNLOCK();
    return ret;
}/*end of CFM_OM_GetFirstHighLevelMdMaBylevelVid*/

 /* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdByIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : md_index - md index
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_GetMdByIndex(
                                UI32_T md_index)
{
    CFM_OM_MD_T *ret_md_p=NULL;
    CFM_OM_LOCK();
    ret_md_p=CFM_OM_LocalGetMdByIndex(md_index);
    CFM_OM_UNLOCK();
    return ret_md_p;
}/*end of CFM_OM_GetMdByIndex*/

 /* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMdByIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the next Md pointer
 * INPUT    : md_index - md index
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_GetNextMdByIndex(
                                    UI32_T md_index)
{
    CFM_OM_MD_T *ret_md_p=NULL;
    CFM_OM_LOCK();
    ret_md_p=CFM_OM_LocalGetNextMdByIndex(md_index);
    CFM_OM_UNLOCK();
    return ret_md_p;
}/*end of CFM_OM_GetNextMdByIndex*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdByLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : level - the md level
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : this will get the first md in the same level
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_GetMdByLevel(
                                CFM_TYPE_MdLevel_T level)
{
    CFM_OM_MD_T *ret_md_p=NULL;
    CFM_OM_LOCK();
    ret_md_p=CFM_OM_LocalGetMdByLevel(level);
    CFM_OM_UNLOCK();
    return ret_md_p;
}/*end of CFM_OM_GetMdByLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMdByLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the md according to the input level.
 * INPUT    : level - md level
 *            index - index of current md and to get the next md
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
*/
CFM_OM_MD_T* CFM_OM_GetNextMdByLevel(
                                    UI32_T md_index,
                                    CFM_TYPE_MdLevel_T level)
{
    CFM_OM_MD_T *ret_md_p=NULL;
    CFM_OM_LOCK();
    ret_md_p=CFM_OM_LocalGetNextMdByLevel(md_index, level);
    CFM_OM_UNLOCK();
    return ret_md_p;
}/*end of CFM_OM_GetNextMdByLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdByName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the Md pointer
 * INPUT    : *md_name_ap - md name
 *            name_len    - md name length
 * OUTPUT   : md pointer
 * RETUEN   : None
 * NOTES    :
 * ------------------------------------------------------------------------
 */
CFM_OM_MD_T* CFM_OM_GetMdByName(
                                UI8_T *md_name_ap,
                                UI32_T name_len)
{
    CFM_OM_MD_T *ret_md_p=NULL;
    CFM_OM_LOCK();
    ret_md_p=CFM_OM_LocalGetMdByName(md_name_ap, name_len);
    CFM_OM_UNLOCK();
    return ret_md_p;
}/*end of CFM_OM_GetMdByName*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewMd
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add a new md to the md link list
 * INPUT    : md_index    - md index
 *            level       - md level
 *            name_length - md name length
 *            *name_ap        - md name pointer
 *            create_way  - the mip create way
 * OUTPUT   : None
 * RETUEN   : TRUE         - success
 *            FALSE        - it has already has this md index
 *                               or it allocate space for store new md
 * NOTES    : Md in the sorted linked list by md index
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewMd(
                        UI32_T md_index,
                        CFM_TYPE_MdLevel_T level,
                        UI16_T name_length,
                        UI8_T *name_ap,
                        CFM_TYPE_MhfCreation_T create_way)
{
    I32_T pre_md_ar_idx=-1, new_md_ar_idx=-1, cur_md_ar_idx= sharmem_data_p->md_head_index;

    CFM_OM_LOCK();
    if(TRUE == CFM_OM_LocalCheckAlreadyExistMdName(name_ap, name_length))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    /*find the position for the new md*/
    while (TRUE == sharmem_data_p->md_ag[cur_md_ar_idx].used
        &&cur_md_ar_idx>=0)
    {
        if(sharmem_data_p->md_ag[cur_md_ar_idx].index == md_index)
        {
            CFM_OM_UNLOCK();
            /*already exist this md*/
            return FALSE;
        }
        else if(sharmem_data_p->md_ag[cur_md_ar_idx].index<md_index)
        {
            pre_md_ar_idx=cur_md_ar_idx;
            cur_md_ar_idx=sharmem_data_p->md_ag[cur_md_ar_idx].next_ar_idx;
        }
        else
        {
            break;
        }
    }

    /*not in the list, insert into the list*/
    if((new_md_ar_idx=CFM_OM_LocalGetFreeMdAllocation())>=0)
    {
        CFM_OM_LocalInitMd(&sharmem_data_p->md_ag[new_md_ar_idx], name_ap, name_length, md_index, level, create_way);
        sharmem_data_p->md_ag[new_md_ar_idx].ar_idx = new_md_ar_idx;

        /*insert at firt md link list*/
        if(pre_md_ar_idx <0)
        {
            sharmem_data_p->md_head_index=new_md_ar_idx;
            sharmem_data_p->md_ag[new_md_ar_idx].next_ar_idx = cur_md_ar_idx;
        }
        else /* insert at middle of end of md link list*/
        {
            sharmem_data_p->md_ag[pre_md_ar_idx].next_ar_idx= new_md_ar_idx;
            sharmem_data_p->md_ag[new_md_ar_idx].next_ar_idx=cur_md_ar_idx;
        }
        CFM_OM_LocalFindNextMdIndex(md_index);
        CFM_OM_UNLOCK();
        return TRUE;
    }

    CFM_OM_UNLOCK();
    return FALSE;
}/*end of CFM_OM_AddNewMd*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMd
 * ------------------------------------------------------------------------
 * PURPOSE  : This function delet a md from the md link list
 * INPUT    : md_index - md index
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMd(
                        UI32_T md_index)
{
    I16_T pre_md=-1, cur_md=sharmem_data_p->md_head_index;

    CFM_OM_LOCK();
    /* find the md*/
    while (TRUE == sharmem_data_p->md_ag[cur_md].used
        &&cur_md>=0)
    {
        if (sharmem_data_p->md_ag[cur_md].index== md_index)
        {
            /*find*/
            break;
        }

        pre_md = cur_md;
        cur_md= sharmem_data_p->md_ag[cur_md].next_ar_idx;
    }

    if(FALSE == sharmem_data_p->md_ag[cur_md].used)
    {
        CFM_OM_UNLOCK();
        /*doesn't have this md*/
        return TRUE;
    }

    /*deleted md is first element*/
    if (pre_md < 0)
    {
        if(sharmem_data_p->md_ag[cur_md].next_ar_idx >=0
            &&TRUE == sharmem_data_p->md_ag[cur_md].used)
        {
            sharmem_data_p->md_head_index=sharmem_data_p->md_ag[cur_md].next_ar_idx;
        }
        else
        {
            sharmem_data_p->md_head_index = -1;
        }
    }
    else
    {
        /*unlink the node and free it*/
        sharmem_data_p->md_ag[pre_md].next_ar_idx = sharmem_data_p->md_ag[cur_md].next_ar_idx;
    }
    sharmem_data_p->md_ag[cur_md].next_ar_idx = -1;
    sharmem_data_p->md_ag[cur_md].used= FALSE;
    sharmem_data_p->md_ag[cur_md].ar_idx = -1;
    sharmem_data_p->md_ag[cur_md].ma_start_ar_idx= -1;

    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_DeleteMd*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMd
 * ------------------------------------------------------------------------
 * PURPOSE  : This function  modify the md content
 * INPUT    : *md_p       - md record pointer
 *            level       - md level
 *            name_length - md name length
 *            *name_ap   - md name pointer
 *            create_way  - the mip create way
 * OUTPUT   :
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMd(
                    CFM_OM_MD_T *md_p,
                    UI8_T level,
                    UI16_T name_length,
                    UI8_T *name_ap,
                    CFM_TYPE_MhfCreation_T create_way)
{

    /*if there are exist ma, it can't change the level*/
    if((md_p->level!=level ) && (md_p->ma_start_ar_idx>=0))
    {
        return FALSE;
    }
    CFM_OM_LOCK();
    md_p->level=level;

    if((NULL == name_ap)||(0==name_length))
    {
        memset(md_p->name_a, 0, CFM_TYPE_MD_MAX_NAME_LENGTH);
        md_p->name_length=0;
    }
    else if(name_length!=md_p->name_length
             ||memcmp(name_ap, md_p->name_a, name_length)) /*name is not match*/
    {
        if(TRUE == CFM_OM_LocalCheckAlreadyExistMdName(name_ap, name_length))
        {
            CFM_OM_UNLOCK();
            return FALSE;
        }

        memset(md_p->name_a, 0, CFM_TYPE_MD_MAX_NAME_LENGTH);
        md_p->name_length = name_length;
        memcpy(md_p->name_a, name_ap, name_length);
    }

    md_p->mhf_creation=create_way;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetMd*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMdMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           create_type - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMdMhfCreation(
                            UI32_T md_index,
                            CFM_TYPE_MhfCreation_T create_type)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    md_p=CFM_OM_LocalGetMdByIndex(md_index);

    if(NULL == md_p)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    md_p->mhf_creation=create_type;
    CFM_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMdMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           send_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMdMhfIdPermission(
                            UI32_T md_index,
                            CFM_TYPE_MhfIdPermission_T send_id_permission)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    md_p=CFM_OM_LocalGetMdByIndex(md_index);

    if(NULL == md_p)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    md_p->mhf_id_permission=send_id_permission;
    CFM_OM_UNLOCK();

    return TRUE;
}/*End of CFM_OM_SetMdMhfIdPermission*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetNextAvailableMdIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next md index
 * INPUT    :
 * OUTPUT   :
 * RETURN   :the next md index can use
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextAvailableMdIndex()
{
    UI32_T ret=0;

    CFM_OM_LOCK();
    ret=sharmem_data_p->md_next_index_g;
    CFM_OM_UNLOCK();

    return ret;
}/*End of CFM_OM_GetNextAvailableMdIndex*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetMaxMaNameLengthInMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the max ma name length in this md
 * INPUT    :
 * OUTPUT   :
 * RETURN   :the max ma name length
 * NOTE     :
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetMaxMaNameLengthInMd(
                                    CFM_OM_MD_T *md_p)
{
    I16_T ma_ar_idx=md_p->ma_start_ar_idx;
    UI32_T max_ma_name_length=0;

    CFM_OM_LOCK();
    while(ma_ar_idx >= 0)
    {
        if(max_ma_name_length < sharmem_data_p->ma_ag[ma_ar_idx].name_length)
        {
            max_ma_name_length = sharmem_data_p->ma_ag[ma_ar_idx].name_length;
        }

        ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }
    CFM_OM_UNLOCK();
    return max_ma_name_length;
}/*end of CFM_OM_GetMaxMaNameLengthInMd*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddNewMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add a new ma
 * INPUT    : md_index     - md index
 *            ma_index     - ma index
 *            ccm_interval - the ccm trasmit interval
 *            name_length  - md name length
 *            *name_ap        - md name
 *            primary_vid  - primary vid of the ma
 *            vid_num      - the vid num in list
 *            vid_list     - the array store the vid list
 *            create_way  - the mip create way
 * OUTPUT   :
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddNewMa(
    UI32_T                  md_index,
    UI32_T                  ma_index,
    UI16_T                  name_length,
    UI8_T                   *name_ap,
    UI16_T                  primary_vid,
    UI32_T                  vid_num,
    UI8_T                   vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1],
    CFM_TYPE_MhfCreation_T  create_way)
{
    CFM_OM_MD_T *md_p= NULL;
    I16_T cur_ma_ar_idx=-1, pre_ma_ar_idx=-1, new_ma_ar_idx=-1;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        /*md doesn't exist*/
        CFM_OM_UNLOCK();
        return FALSE;
    }

    if (TRUE == CFM_OM_LocalIsMaNameExist(md_p, name_ap, name_length))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    if(TRUE == CFM_OM_LocalCheckAllMaExistVid(md_p, NULL, vid_list))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    /*find the ma*/
    cur_ma_ar_idx=md_p->ma_start_ar_idx;

    while (cur_ma_ar_idx >= 0
        &&TRUE == sharmem_data_p->ma_ag[cur_ma_ar_idx].used)
    {
        if(sharmem_data_p->ma_ag[cur_ma_ar_idx].index == ma_index)
        {
            CFM_OM_UNLOCK();
            /*already exist the ma*/
            return FALSE;
        }
        else if(sharmem_data_p->ma_ag[cur_ma_ar_idx].index<ma_index)
        {
            pre_ma_ar_idx=cur_ma_ar_idx;
            cur_ma_ar_idx=sharmem_data_p->ma_ag[cur_ma_ar_idx].next_ar_idx;
        }
        else
        {
            break;
        }
    }

    /*insert in to the ma link list*/
    if ((new_ma_ar_idx=CFM_OM_LocalGetFreeMaAllocation()) >=0)
    {
        CFM_OM_LocalInitMa(&sharmem_data_p->ma_ag[new_ma_ar_idx], ma_index, name_ap, name_length, md_p, create_way);
        sharmem_data_p->ma_ag[new_ma_ar_idx].ar_idx = new_ma_ar_idx;

        memcpy(sharmem_data_p->ma_ag[new_ma_ar_idx].vid_bitmap_a, vid_list, (SYS_DFLT_DOT1QMAXVLANID/8)+1);
        sharmem_data_p->ma_ag[new_ma_ar_idx].primary_vid=primary_vid;
        sharmem_data_p->ma_ag[new_ma_ar_idx].num_of_vids = vid_num;

        /*be the first element of ma list*/
        if( pre_ma_ar_idx < 0)
        {
            md_p->ma_start_ar_idx = new_ma_ar_idx;
            sharmem_data_p->ma_ag[new_ma_ar_idx].next_ar_idx= cur_ma_ar_idx;
        }
        else
        {
            sharmem_data_p->ma_ag[pre_ma_ar_idx].next_ar_idx= new_ma_ar_idx;
            sharmem_data_p->ma_ag[new_ma_ar_idx].next_ar_idx= cur_ma_ar_idx;
        }
    }
    else
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    md_p->next_available_ma_index=CFM_OM_LocalFindNextMaIndex(md_index, ma_index);

    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_AddNewMa*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function delete a ma
 * INPUT    : md_index   - md index
 *            ma_index   - ma index
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMa(
                    UI32_T md_index,
                    UI32_T ma_index)
{
    CFM_OM_MD_T *md_p=NULL;
    I16_T ma_ar_idx=-1, pre_ma_ar_idx=-1;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        /*md doesn't exist*/
        return FALSE;
    }

    /*find the ma*/
    ma_ar_idx=md_p->ma_start_ar_idx;
    pre_ma_ar_idx=md_p->ma_start_ar_idx;

    while(TRUE == sharmem_data_p->ma_ag[ma_ar_idx].used
        &&ma_ar_idx>=0)
    {
        if(sharmem_data_p->ma_ag[ma_ar_idx].index == ma_index)
        {
            break;
        }

        pre_ma_ar_idx=ma_ar_idx;
        ma_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }

    if(ma_ar_idx < 0)
    {
        CFM_OM_UNLOCK();
        /*doesn't have this ma*/
        return TRUE;
    }

    /*delet the ma at first member*/
    if(pre_ma_ar_idx == ma_ar_idx)
    {
        md_p->ma_start_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }
    else
    {
        sharmem_data_p->ma_ag[pre_ma_ar_idx].next_ar_idx=sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx;
    }

    /*free the ma memory*/
    sharmem_data_p->ma_ag[ma_ar_idx].used=FALSE;
    sharmem_data_p->ma_ag[ma_ar_idx].next_ar_idx=-1;
    sharmem_data_p->ma_ag[ma_ar_idx].ar_idx = -1;

    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_DeleteMa*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function delete a ma
 * INPUT    : md_index   - md index
 *            ma_index   - ma index
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteMaVlan(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI16_T vid)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    ma_p->vid_bitmap_a[(vid-1)/8] &=(~(1 << (7 - ((vid-1)%8))));
    if (ma_p->num_of_vids > 0)
        ma_p->num_of_vids --;

    CFM_OM_UNLOCK();

    return TRUE;
}/*End of CFM_OM_DeleteMaVlan*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma name
 * INPUT    : *md_p        - the md record pointer
 *            *ma_p        - the ma record pointer
 *            *name_ap     - the m name array pointer
 *            name_len     - md name length
 *            primary_vid  - the primary vid of the ma
 *            vid_num      - the vid number is vid_list
 *            vlid_list    - the vid lists store the vids
 *            create_way  - the mip create way
 * OUTPUT   :
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMa(
    CFM_OM_MD_T             *md_p,
    CFM_OM_MA_T             *ma_p,
    UI8_T                   *name_ap,
    UI32_T                  name_len,
    UI16_T                  primary_vid,
    UI32_T                  vid_num,
    UI8_T                   vid_list[(SYS_DFLT_DOT1QMAXVLANID/8)+1],
    CFM_TYPE_MhfCreation_T  create_way)
{
    CFM_OM_LOCK();
    /*if it want to change ma name*/
    if(memcmp(ma_p->name_a, name_ap, name_len)
       || name_len!=ma_p->name_length)
    {
        if (TRUE == CFM_OM_LocalIsMaNameExist(md_p, name_ap, name_len))
        {
            CFM_OM_UNLOCK();
            return FALSE;
        }
    }

    memset(ma_p->name_a, 0, CFM_TYPE_MA_MAX_NAME_LENGTH);
    ma_p->name_length=name_len;
    memcpy(ma_p->name_a, name_ap, name_len);

    /*modify vlan list and primary vid*/
    if((0!=primary_vid)&&(TRUE == CFM_OM_LocalCheckAllMaExistVid(md_p, ma_p, vid_list)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    /*remove all the exist vid in ma*/
    memset(ma_p->vid_bitmap_a, 0, (SYS_DFLT_DOT1QMAXVLANID/8)+1);
    memcpy(ma_p->vid_bitmap_a, vid_list, (SYS_DFLT_DOT1QMAXVLANID/8)+1);

    ma_p->primary_vid=primary_vid;
    ma_p->mhf_creation=create_way;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetMa*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMaMhfCreation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma mhf creation type
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            create_type -the mhf cration type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *            CFM_TYPE_MHF_CREATION_NONE
 *            CFM_TYPE_MHF_CREATION_DEFAULT
 *            CFM_TYPE_MHF_CREATION_EXPLICIT
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaMhfCreation(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_MhfCreation_T create_type)
{
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    ma_p=CFM_OM_LocalGetMa(md_index, ma_index);

    if(NULL == ma_p)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    ma_p->mhf_creation=create_type;
    CFM_OM_UNLOCK();

    return TRUE;
}/*End of CFM_OM_SetMaMhfCreation*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMaMhfIdPermission
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the mhf cration type
 * INPUT    : md_index  - the md index
 *           ma_index   - the ma index
 *           mhf_id_permission - the send id permission
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *            CFM_TYPE_MHF_ID_PERMISSION_NONE=1,
 *            CFM_TYPE_MHF_ID_PERMISSION_CHASSIS,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_MANAGE,
 *            CFM_TYPE_MHF_ID_PERMISSION_SENDID_DEFER
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaMhfIdPermission(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_MhfIdPermission_T mhf_id_permission)
{
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    ma_p=CFM_OM_LocalGetMa(md_index, ma_index);

    if(NULL == ma_p)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    ma_p->mhf_id_permission=mhf_id_permission;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetMaMhfIdPermission*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_SetMaNumOfVids
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the ma can have more than on vid
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            vid_num   - can have more than one vid
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaNumOfVids(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T vid_num)
{
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    ma_p= CFM_OM_LocalGetMa(md_index, ma_index);
    if(NULL == ma_p)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    ma_p->num_of_vids=vid_num;
    CFM_OM_UNLOCK();

    return TRUE;
}/*End of CFM_OM_SetMaNumOfVids */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : this function set the ma name
 * INPUT    : md_p         - the md pointer
 *            ma_p         - the ma pointer
 *            name_ap      - the ma name array pointer
 *            name_length  - the ma name length
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaName(
    CFM_OM_MD_T *md_p,
    CFM_OM_MA_T *ma_p,
    UI8_T       *name_ap,
    UI32_T      name_length)
{
    BOOL_T  ret = FALSE;

    CFM_OM_LOCK();

    /* check if new name is the same as the old one
     */
    if (  (ma_p->name_length != name_length)
        ||(memcmp(ma_p->name_a, name_ap, name_length))
       )
    {
        /* check if ma name already exists */
        if (FALSE == CFM_OM_LocalIsMaNameExist(md_p, name_ap, name_length))
        {
            if (CFM_TYPE_MA_NAME_ICC_BASED == ma_p->format)
            {
                if (name_length <= CFM_TYPE_MA_MAX_NAME_LENGTH_FOR_Y1731)
                    ret = TRUE;
            }
            else
            {
                ret = TRUE;
            }

            if (TRUE == ret)
            {
                memset(ma_p->name_a, 0, CFM_TYPE_MA_MAX_NAME_LENGTH);
                memcpy(ma_p->name_a, name_ap, name_length);
                ma_p->name_length=name_length;
            }
        }
    }
    CFM_OM_UNLOCK();

    return ret;
}/*End of CFM_OM_SetMaName*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMaNameFormat
 *-------------------------------------------------------------------------
 * PURPOSE  : To set the name format of MA
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            name_format - the name format
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : if name_format == CFM_TYPE_MA_NAME_ICC_BASED
 *               name_length must <= 13
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaNameFormat(
    UI32_T              md_index,
    UI32_T              ma_index,
    CFM_TYPE_MA_Name_T  name_format)
{
    CFM_OM_MA_T *ma_p = NULL;
    BOOL_T      ret   = FALSE;

    CFM_OM_LOCK();
    ma_p=CFM_OM_LocalGetMa(md_index, ma_index);
    if (NULL != ma_p)
    {
        if (CFM_TYPE_MA_NAME_ICC_BASED == name_format)
        {
            if (ma_p->name_length <= 13)
            {
                ret = TRUE;
            }
        }
        else
        {
            ret = TRUE;
        }

        if (TRUE == ret)
        {
            ma_p->format = name_format;
        }
    }
    CFM_OM_UNLOCK();
    return ret;
}/*End of CFM_OM_SetMaNameFormat*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMaCCInterval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma ccm interval
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            interval - the ccm trasmit interval
 * OUTPUT   :
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMaCCInterval(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI8_T interval)
{
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    if(NULL == (ma_p = CFM_OM_LocalGetMa(md_index, ma_index)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    ma_p->ccm_interval=interval;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetMaCCInterval*/

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMaCCInterval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma ccm interval
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *interval   - the ma ccm interval pointer
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTE     : if ma_name_len specified, it will use ma name to be the key
 *                of it will use the ma_index to be the key
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMaCCInterval(
                              UI32_T md_index,
                              UI32_T ma_index,
                              UI8_T *ma_name_ap,
                              UI32_T name_len,
                              CFM_TYPE_CcmInterval_T *interval)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    BOOL_T ret=TRUE;

    CFM_OM_LOCK();
    if(name_len!=0)
    {
        if(FALSE == CFM_OM_LocalGetMdMaByMaName(ma_name_ap,  name_len, &md_p, &ma_p))
        {
            ret=FALSE;
        }
    }
    else if((md_index>0)&&(ma_index >0))
    {
        if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index,&md_p,&ma_p))
        {
            ret=FALSE;
        }
    }

    if(FALSE == ret)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *interval = ma_p->ccm_interval;
    CFM_OM_UNLOCK();

    return TRUE;
}/*end of CFM_OM_GetMaCCInterval*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaCcInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next ccm interval
 * INPUT    : md_index     - the md index
 *            *ma_index_p  - the ma index
 *            *ma_name_ap  - the ma name array pointer
 *            aname_len    - the ma name length
 * OUTPUT   : *interval_p  - the ccm interval
 *            *ma_index_p  - the next ma index
 *            *ma_name_ap  - the name ma name array pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if ma_name_len specified, it will use ma name to be the key
 *                of it will use the ma_index to be the key
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMaCcInterval(
                             UI32_T md_index,
                             UI32_T *ma_index_p,
                             UI8_T *ma_name_ap,
                             UI32_T name_len,
                             CFM_TYPE_CcmInterval_T *interval_p)
{
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    if(name_len!=0)
    {
        if(NULL != (ma_p= CFM_OM_LocalGetNextMaByMaName(md_index, ma_name_ap, name_len)))
        {
            memcpy(ma_name_ap, ma_p->name_a, ma_p->name_length);
        }
    }
    else if((md_index>0)&&(ma_index_p >0))
    {
        if(NULL != (ma_p= CFM_OM_LocalGetNextMaByMaIndex( md_index, *ma_index_p)))
        {
            *ma_index_p = ma_p->index;
        }
    }

    /*if doesn't find the next ma*/
    if(NULL==ma_p)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *interval_p = ma_p->ccm_interval;
    CFM_OM_UNLOCK();

    return TRUE;
}/*End of CFM_OM_GetNextMaCcInterval*/

#if 0 //not used at present
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMaCcStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the CCM status
 * INPUT    : md_index    - the md index
 *            ma_index    - the ma index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *status_p   - the ccm status
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if ma_name_len specified, it will use ma name to be the key
 *                of it will use the ma_index to be the key
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMaCcStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI8_T *ma_name_ap,
                            UI32_T name_len,
                            CFM_TYPE_CcmStatus_T *status_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    BOOL_T ret=TRUE;

    CFM_OM_LOCK();
    if(name_len != 0)
    {
        if(FALSE == CFM_OM_LocalGetMdMaByMaName(ma_name_ap,  name_len, &md_p, &ma_p))
        {
            ret=FALSE;
        }
    }
    else if((ma_index >0)&&(md_index>0))
    {
        if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p,&ma_p))
        {
            ret=FALSE;
        }
    }

    if(FALSE == ret)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    *status_p =ma_p->ccm_status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetMaCcStatus*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaCcStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next ccm status
 * INPUT    : md_index    - the md index
 *            *ma_index   - the ma_index
 *            *ma_name_ap - the ma name array pointer
 *            name_len    - the ma name length
 * OUTPUT   : *status_p   - the ccm status
 *            *ma_index   - the next ma index
 *            *ma_name_ap - the next ma name array pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if ma_name_len specified, it will use ma name to be the key
 *                of it will use the ma_index to be the key
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMaCcStatus(
                           UI32_T md_index,
                           UI32_T *ma_index,
                           UI8_T *ma_name_ap,
                           UI32_T name_len,
                           CFM_TYPE_CcmStatus_T *status_p)
{
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    if(name_len!=0)
    {
        if(NULL != (ma_p= CFM_OM_LocalGetNextMaByMaName(md_index, ma_name_ap, name_len)))
        {
            memcpy(ma_name_ap, ma_p->name_a, ma_p->name_length);
        }
    }
    else if((md_index>0)&&(ma_index >0))
    {
        if(NULL != (ma_p= CFM_OM_LocalGetNextMaByMaIndex(md_index, *ma_index)))
        {
            *ma_index = ma_p->index;
        }
    }

    if(NULL == ma_p)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *status_p = ma_p->ccm_status;
    CFM_OM_UNLOCK();

    return TRUE;
}/*End of CFM_OM_GetNextMaCcStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMa
 * ------------------------------------------------------------------------
 * PURPOSE  : This function modify the ma content
 * INPUT    : md_index - md index
 *            ma_index - ma index
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_MA_T* CFM_OM_GetMa(
                        UI32_T md_index,
                        UI32_T ma_index)
{
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    ma_p = CFM_OM_LocalGetMa(md_index, ma_index);
    CFM_OM_UNLOCK();
    return ma_p;
}/*end of CFM_OM_GetMa*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaByMaName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the next ma pointer
 * INPUT    : md_index - md index
 *            ma_index - ma index
 * OUTPUT   : ma pointer
 * RETUEN   : None
 * NOTES    : This get next ma is get the next ma under the md
 * ------------------------------------------------------------------------
 */
CFM_OM_MA_T* CFM_OM_GetNextMaByMaIndex(
                                    UI32_T md_index,
                                    UI32_T ma_index)
{
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    ma_p=CFM_OM_LocalGetNextMaByMaIndex(md_index, ma_index);
    CFM_OM_UNLOCK();
    return ma_p;
}/*end of CFM_OM_GetNextMaByMaIndex*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMaxMdLevelOfAllMd
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the max level
 * INPUT    : None
 * OUTPUT   : max level of all md
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_TYPE_MdLevel_T CFM_OM_GetMaxMdLevel( )
{
    CFM_TYPE_MdLevel_T max_level=CFM_TYPE_MD_LEVEL_NONE;
    I16_T md_ar_idx=sharmem_data_p->md_head_index;

    CFM_OM_LOCK();
    while(TRUE == sharmem_data_p->md_ag[md_ar_idx].used
        &&md_ar_idx >= 0)
    {
        if(sharmem_data_p->md_ag[md_ar_idx].level>max_level)
        {
            max_level= sharmem_data_p->md_ag[md_ar_idx].level;
         }

        md_ar_idx=sharmem_data_p->md_ag[md_ar_idx].next_ar_idx;
    }
    CFM_OM_UNLOCK();

    return max_level;
}/*end of CFM_OM_GetMaxMdLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddToLtrQueue
 * ------------------------------------------------------------------------
 * PURPOSE  : the function add the ltr which will reply to the sender into the queue
 * INPUT    : *ltr_queue_p  -the element which want to put into list
 * OUTPUT   : None
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddToLtrQueue(
                            CFM_OM_LTR_QueueElement_T *ltr_queue_p)
{
    CFM_OM_LTR_QUEUE_HEAD_T *ltr_queue_head=CFM_OM_GetLtrQueueHeadPtr();

    CFM_OM_LOCK();
    if(NULL == ltr_queue_head->first_pdu_p)
    {
        ltr_queue_head->first_pdu_p=ltr_queue_p;
        ltr_queue_head->last_pdu_p=ltr_queue_p;
        ltr_queue_p->next_element_p=NULL;

        CFM_OM_UNLOCK();
        return TRUE;
    }

    ltr_queue_head->last_pdu_p->next_element_p=ltr_queue_p;
    ltr_queue_head->last_pdu_p=ltr_queue_p;

    ltr_queue_head->enqueued_LTRs++;

    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_AddToLtrQueue*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRemoveFirstElementFromLTrQueue
 * ------------------------------------------------------------------------
 * PURPOSE  : the function get the first element and unlink the first element from list
 * INPUT    : **ltr_queue_pp - the element pointer which get from list
 * OUTPUT   : None
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    : This function won't free the return pointer
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetRemoveFirstElementFromLTrQueue(
                                CFM_OM_LTR_QueueElement_T **ltr_queue_pp)
{
    CFM_OM_LTR_QUEUE_HEAD_T *ltr_queue_head=CFM_OM_GetLtrQueueHeadPtr();
    CFM_OM_LTR_QueueElement_T *temp_element_p;

    if(NULL==ltr_queue_head->first_pdu_p)
    {
        return FALSE;
    }

    CFM_OM_LOCK();
    temp_element_p=ltr_queue_head->first_pdu_p;

    if(ltr_queue_head->first_pdu_p == ltr_queue_head->last_pdu_p)
    {
        ltr_queue_head->first_pdu_p=NULL;
        ltr_queue_head->last_pdu_p=NULL;
    }
    else
    {
        ltr_queue_head->first_pdu_p=temp_element_p->next_element_p;
    }

    ltr_queue_head->enqueued_LTRs --;
    *ltr_queue_pp=temp_element_p;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetRemoveFirstElementFromLTrQueue*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_DeleteAllLTrQueueElement
 * ------------------------------------------------------------------------
 * PURPOSE  :This function will delet and free all pending element in queue
 * INPUT    None
 * OUTPUT   : None
 * RETUEN   : TRUE          - success
 *            FALSE         - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_DeleteAllLTrQueueElement()
{
    CFM_OM_LTR_QUEUE_HEAD_T *ltr_queue_head=CFM_OM_GetLtrQueueHeadPtr();
    CFM_OM_LTR_QueueElement_T *cur_element_p,*free_element_p;

    if(NULL==ltr_queue_head->first_pdu_p)
    {
        return TRUE;
    }
    CFM_OM_LOCK();
    cur_element_p=ltr_queue_head->first_pdu_p;

    while(NULL!=cur_element_p)
    {
        free_element_p=cur_element_p;
        cur_element_p=cur_element_p->next_element_p;

        L_MM_Free(free_element_p);
    }

    ltr_queue_head->first_pdu_p=NULL;
    ltr_queue_head->last_pdu_p=NULL;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_DeleteAllLTrQueueElement */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLtrQueueHeadPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : the funciton will return the ltr queue header ptr
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_LTR_QUEUE_HEAD_T * CFM_OM_GetLtrQueueHeadPtr()
{
    return &sharmem_data_p->ltr_queue_header_g;
}/*end of CFM_OM_GetLtrQueueHeadPtr*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetCrossCheckStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck status.
 * INPUT    : status - the cross check status
 *            ma_p   - the ma pointer
 * OUTPUT   : None
 * RETUEN   : TRUE   - success
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetCrossCheckStatus(
                            CFM_TYPE_CrossCheckStatus_T status,
                            CFM_OM_MA_T *ma_p)
{
    CFM_OM_LOCK();
    ma_p->cross_check_status = status;
    CFM_OM_UNLOCK();

    return TRUE;
}/*end of CFM_OM_SetCrossCheckStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCrossCheckStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the crosscheck status.
 * INPUT    : *ma_name_ap - the ma name array pointer
 *            *ma_len     - the ma length
 * OUTPUT   : *status_p   - cross check status
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCrossCheckStatus(
                              CFM_TYPE_CrossCheckStatus_T *status_p,
                              UI8_T *ma_name_ap,
                              UI32_T ma_len)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMaName(ma_name_ap, ma_len, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *status_p = ma_p->cross_check_status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetCrossCheckStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetCrossCheckStartDelay
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : delay - the start delay time
 * OUTPUT   : None
 * RETUEN   : TRUE  - success
 *            FALSE - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetCrossCheckStartDelay(
                            UI32_T delay)
{
    CFM_OM_LOCK();
    sharmem_data_p->global_cfg_g.remote_mep_start_delay=delay;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetCrossCheckStartDelay*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCrossCheckStartDelay
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    :
 * OUTPUT   : *delay_p - the start delay time
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCrossCheckStartDelay(
                                UI32_T *delay_p)
{
    BOOL_T  ret = FALSE;

    if (NULL != delay_p)
    {
        CFM_OM_LOCK();
        *delay_p = sharmem_data_p->global_cfg_g.remote_mep_start_delay;
        CFM_OM_UNLOCK();
        ret = TRUE;
    }
    return ret;
}/*end of CFM_OM_GetCrossCheckStartDelay*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetLinkTraceCacheStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : status   - the linktrace cache status
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetLinkTraceCacheStatus(
                            CFM_TYPE_LinktraceStatus_T status)
{
    CFM_OM_LOCK();
    sharmem_data_p->global_cfg_g.link_trace_cache_status=status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetLinkTraceCacheStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLinkTraceCacheStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : None
 * OUTPUT   : *status_p- the link trace cache status
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T  CFM_OM_GetLinkTraceCacheStatus(
                            CFM_TYPE_LinktraceStatus_T *status_p)
{
    BOOL_T  ret = FALSE;

    if (NULL != status_p)
    {
        CFM_OM_LOCK();
        *status_p=sharmem_data_p->global_cfg_g.link_trace_cache_status;
        CFM_OM_UNLOCK();
        ret = TRUE;
    }
    return ret;
}/*end of CFM_OM_GetLinkTraceCacheStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetLinkTraceCacheSize
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : size     - the link trace cache size
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetLinkTraceCacheSize(
                                UI32_T size)
{
    CFM_OM_LOCK();
    sharmem_data_p->global_cfg_g.link_trace_size=size;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetLinkTraceCacheSize*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLinkTraceCacheSize
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : None
 * OUTPUT   : *size_p  - the link trace cache size
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetLinkTraceCacheSize(
                            UI32_T *size_p)
{
    BOOL_T  ret = FALSE;

    if (NULL != size_p)
    {
        CFM_OM_LOCK();
        *size_p = sharmem_data_p->global_cfg_g.link_trace_size;
        CFM_OM_UNLOCK();
        ret = TRUE;
    }
    return ret;
}/*end of CFM_OM_GetLinkTraceCacheSize*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetLinkTraceCacheHoldTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : hold_time - the link trace cache hold time
 * OUTPUT   : None
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetLinkTraceCacheHoldTime(
                                UI32_T hold_time)
{
    CFM_OM_LOCK();
    sharmem_data_p->global_cfg_g.link_trace_hold_time=hold_time;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetLinkTraceCacheHoldTime*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLinkTraceCacheHoldTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    :  None
 * OUTPUT   : *hold_time_p- the link trace cache hold time pointer
 * RETUEN   : TRUE        - success
 *            FALSE       - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetLinkTraceCacheHoldTime(
                                    UI32_T *hold_time_p)
{
    BOOL_T  ret = FALSE;

    if (NULL != hold_time_p)
    {
        CFM_OM_LOCK();
        *hold_time_p= sharmem_data_p->global_cfg_g.link_trace_hold_time;
        CFM_OM_UNLOCK();
        ret = TRUE;
    }
    return ret;
}/*end of CFM_OM_GetLinkTraceCacheHoldTime*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetCFMGlobalStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : status   - the CFM global status
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetCFMGlobalStatus(
                                CFM_TYPE_CfmStatus_T status)
{
    CFM_OM_LOCK();
    sharmem_data_p->global_cfg_g.cfm_status=status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetCFMGlobalStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCFMGlobalStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    :
 * OUTPUT   : *status_p  - the CFM global status
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCFMGlobalStatus(
                                CFM_TYPE_CfmStatus_T *status_p)
{
    BOOL_T  ret = FALSE;

    if (NULL != status_p)
    {
        CFM_OM_LOCK();
        *status_p=sharmem_data_p->global_cfg_g.cfm_status;
        CFM_OM_UNLOCK();
        ret = TRUE;
    }
    return ret;
}/*end of CFM_OM_GetCFMGlobalStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetCFMPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : lport  - the logical port
 *            status - the CFM port status
 * OUTPUT   : None
 * RETUEN   : TRUE   - success
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetCFMPortStatus(
                            UI32_T lport,
                            CFM_TYPE_CfmStatus_T status)
{
    CFM_OM_LOCK();
    sharmem_data_p->cfm_port_status_ag[lport-1]=status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetCFMPortStatus */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCFMPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : lport    - the logical port
 * OUTPUT   : *status_p- the CFM port status
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCFMPortStatus(
                            UI32_T lport,
                            CFM_TYPE_CfmStatus_T *status_p)
{
    if ((0 == lport) || (NULL == status_p))
    {
        return FALSE;
    }

    CFM_OM_LOCK();
    *status_p=sharmem_data_p->cfm_port_status_ag[lport-1];
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetCFMPortStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetArchiveHoldTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index   - the md index
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetArchiveHoldTime(
                                UI32_T md_index,
                                UI32_T hold_time)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        /*md doesn't exist*/
        return FALSE;
    }

    md_p->mep_achive_holdtime=hold_time;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetArchiveHoldTime*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetArchiveHoldTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index   - the md index
 * OUTPUT   : *hold_time_p - the archive hold time
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetArchiveHoldTime(
                                UI32_T md_index,
                                UI32_T *hold_time_p)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        /*md doesn't exist*/
        return FALSE;
    }

    *hold_time_p=md_p->mep_achive_holdtime;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetArchiveHoldTime*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetMdLowPrDef
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the lowest priotiry on md
 * INPUT    : md_index   - the md index
 *            priority   - the lowest fault notify priority
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetMdLowPrDef(
                            UI32_T md_index,
                            CFM_TYPE_FNG_LowestAlarmPri_T priority)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        /*md doesn't exist*/
        return FALSE;
    }

    md_p->low_pri_def = priority;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetMdLowPrDef*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFaultNotifyLowestPriority
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index - the md index
 * OUTPUT   : *priority_p - loweset priority
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetFaultNotifyLowestPriority(
                            UI32_T md_index,
                            CFM_TYPE_FNG_LowestAlarmPri_T *priority_p)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        /*md doesn't exist*/
        return FALSE;
    }

    *priority_p= md_p->low_pri_def;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetFaultNotifyLowestPriority*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetFaultNotifyAlarmTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index   - the md index
 *            alarm_time - the fault notify alarm time
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetFaultNotifyAlarmTime(
                                    UI32_T md_index,
                                    UI32_T alarm_time)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        /*md doesn't exist*/
        return FALSE;
    }

    md_p->fng_alarm_time = alarm_time;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetFaultNotifyAlarmTime*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFaultNotifyAlarmTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index    - the md index
 * OUTPUT   : *alarm_time_p - the fault notify alarm time pointer in second
 * RETUEN   : TRUE        - success
 *            FALSE       - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetFaultNotifyAlarmTime(
                                UI32_T md_index,
                                UI32_T *alarm_time_p)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        /*md doesn't exist*/
        return FALSE;
    }

    *alarm_time_p= md_p->fng_alarm_time;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetFaultNotifyAlarmTime*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetFaultNotifyRestTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index   - the md index
 *            reset_time - the fault notify reset time in second
 * OUTPUT   : None
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetFaultNotifyRestTime(
                                UI32_T md_index,
                                UI32_T reset_time)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        /*md doesn't exist*/
        return FALSE;
    }

    md_p->fng_reset_time = reset_time;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetFaultNotifyRestTime*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetFaultNotifyResetTime
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the crosscheck start delay.
 * INPUT    : md_index    - the md index
 * OUTPUT   : *reset_time_p - the fault notify reset time in second
 * RETUEN   : TRUE        - success
 *            FALSE       - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetFaultNotifyResetTime(
                                    UI32_T md_index,
                                    UI32_T *reset_time_p)
{
    CFM_OM_MD_T *md_p=NULL;

    CFM_OM_LOCK();
    /*find the md*/
    if (NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        /*md doesn't exist*/
        return FALSE;
    }

    *reset_time_p = md_p->fng_reset_time;
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetFaultNotifyResetTime*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_ClearErrorsList
 * ------------------------------------------------------------------------
 * PURPOSE  : This function clear the error list
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_ClearErrorsList()
{
    CFM_OM_ErrorListHead_T *error_list_head = CFM_OM_LocalGetErrorListHeadPtr();

    CFM_OM_LOCK();
    error_list_head->first_ar_idx=-1;
    error_list_head->last_ar_idx=-1;
    error_list_head->errors_number=0;

    memset(sharmem_data_p->error_list_ag, 0, sizeof(sharmem_data_p->error_list_ag));
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_ClearErrorsList*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_RemoveErrorByLevel
 * ------------------------------------------------------------------------
 * PURPOSE  : This function remove the error form the error list accrodin to the domain level
 * INPUT    : level - the md level
 * OUTPUT   : None
 * RETUEN   : TRUE  - sucess
 *            FALSE - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_RemoveErrorByLevel(
                                CFM_TYPE_MdLevel_T level)
{
    CFM_OM_ErrorListHead_T *error_list_head;
    I16_T cur_err_ar_idx,pre_err_ar_idx;

    CFM_OM_LOCK();
    error_list_head = CFM_OM_LocalGetErrorListHeadPtr();
    cur_err_ar_idx=error_list_head->first_ar_idx;
    pre_err_ar_idx=error_list_head->first_ar_idx;

    while( cur_err_ar_idx >=0
        &&TRUE == sharmem_data_p->error_list_ag[cur_err_ar_idx].used)
    {
        if(sharmem_data_p->error_list_ag[cur_err_ar_idx].level == level)
        {
            /*remove first node*/
            if(cur_err_ar_idx==error_list_head->first_ar_idx)
            {
                error_list_head->first_ar_idx = sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
                pre_err_ar_idx=error_list_head->first_ar_idx;

                /*there are not node*/
                if(-1 == error_list_head->first_ar_idx)
                {
                    error_list_head->last_ar_idx= -1;
                }

                error_list_head->errors_number --;
                sharmem_data_p->error_list_ag[cur_err_ar_idx].used=FALSE;
                cur_err_ar_idx=pre_err_ar_idx;
                continue;
            }
            /*remove last node*/
            else if(cur_err_ar_idx == error_list_head->last_ar_idx)
            {
                sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=-1;
                error_list_head->last_ar_idx=pre_err_ar_idx;
            }
            /*remove middle node*/
            else
            {
                sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
            }

            error_list_head->errors_number --;
            sharmem_data_p->error_list_ag[cur_err_ar_idx].used=FALSE;
            cur_err_ar_idx=sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx;

            continue;
        }
        pre_err_ar_idx=cur_err_ar_idx;
        cur_err_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_RemoveErrorByLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_RemoveErrorByAis
 * ------------------------------------------------------------------------
 * PURPOSE  : This function remove the error form the error list accrodin
 *              to the specified mep id, md name and ma name.
 * INPUT    : mep_id      - the mep id
 *            md_name_p   - pointer to md name
 *            md_name_len - md name length
 *            ma_name_p   - pointer to ma name
 *            ma_name_len - ma name length
 * OUTPUT   : None
 * RETUEN   : TRUE  - sucess
 *            FALSE - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_RemoveAisErrorByMepMa(
    UI32_T  mep_id,
    UI8_T   *md_name_p,
    UI32_T  md_name_len,
    UI8_T   *ma_name_p,
    UI32_T  ma_name_len)
{
    CFM_OM_ErrorListHead_T *error_list_head;
    I16_T cur_err_ar_idx=-1, pre_err_ar_idx=-1;

    CFM_OM_LOCK();
    error_list_head=CFM_OM_LocalGetErrorListHeadPtr();
    cur_err_ar_idx=error_list_head->first_ar_idx;
    pre_err_ar_idx=error_list_head->first_ar_idx;

    while( cur_err_ar_idx >=0
        &&TRUE == sharmem_data_p->error_list_ag[cur_err_ar_idx].used)
    {
        if(sharmem_data_p->error_list_ag[cur_err_ar_idx].mp_id == mep_id
            &&sharmem_data_p->md_ag[sharmem_data_p->error_list_ag[cur_err_ar_idx].md_ar_idx].name_length==md_name_len
            &&!memcmp(sharmem_data_p->md_ag[sharmem_data_p->error_list_ag[cur_err_ar_idx].md_ar_idx].name_a, md_name_p, md_name_len)
            &&sharmem_data_p->ma_ag[sharmem_data_p->error_list_ag[cur_err_ar_idx].ma_ar_idx].name_length==ma_name_len
            &&!memcmp(sharmem_data_p->ma_ag[sharmem_data_p->error_list_ag[cur_err_ar_idx].ma_ar_idx].name_a, ma_name_p, ma_name_len)
            &&(sharmem_data_p->error_list_ag[cur_err_ar_idx].reason_bit_map&(BIT_4)) /*AIS*/
           )
        {
            sharmem_data_p->error_list_ag[cur_err_ar_idx].reason_bit_map&=(~BIT_4);

            if(0!= sharmem_data_p->error_list_ag[cur_err_ar_idx].reason_bit_map)
            {/*if there is not only AIS error exit, don't clear the node*/
                continue;
            }

            /*remove first node*/
            if(cur_err_ar_idx==error_list_head->first_ar_idx)
            {
                error_list_head->first_ar_idx = sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
                pre_err_ar_idx=error_list_head->first_ar_idx;

                /*there are no node*/
                if(-1 == error_list_head->first_ar_idx)
                {
                    error_list_head->last_ar_idx= -1;
                }

                error_list_head->errors_number --;
                sharmem_data_p->error_list_ag[cur_err_ar_idx].used=FALSE;
                cur_err_ar_idx=pre_err_ar_idx;

                continue;
            }
            /*remove last node*/
            else if(cur_err_ar_idx == error_list_head->last_ar_idx)
            {
                sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=-1;
                error_list_head->last_ar_idx=pre_err_ar_idx;
            }
            /*remove middle node*/
            else
            {
                sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
            }

            error_list_head->errors_number --;
            sharmem_data_p->error_list_ag[cur_err_ar_idx].used=FALSE;
            cur_err_ar_idx=sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx;

            continue;
        }

        pre_err_ar_idx=cur_err_ar_idx;
        cur_err_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;

    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_RemoveErrorByLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_RemoveErrorByDomainName
 * ------------------------------------------------------------------------
 * PURPOSE  : This function remove the error form the error list accrodin to the domain names
 * INPUT    : *name_ap  - the md name array pointer
 *            name_len  - the md name length
 * OUTPUT   : None
 * RETUEN   : TRUE      - sucess
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_RemoveErrorByDomainName(
                                UI8_T *name_ap,
                                UI32_T name_len)
{
    CFM_OM_ErrorListHead_T *error_list_head=CFM_OM_LocalGetErrorListHeadPtr();
    I16_T cur_err_ar_idx=-1, pre_err_ar_idx=-1;
    CFM_OM_MD_T *md_p=NULL, *error_md_p=NULL;
    CFM_OM_MA_T *error_ma_p=NULL;

    CFM_OM_LOCK();
    if(NULL == (md_p=CFM_OM_LocalGetMdByName(name_ap, name_len)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    cur_err_ar_idx=error_list_head->first_ar_idx;
    pre_err_ar_idx=error_list_head->first_ar_idx;

    while(cur_err_ar_idx>=0
        &&TRUE == sharmem_data_p->error_list_ag[cur_err_ar_idx].used)
    {
        if(TRUE == CFM_OM_LocalGetMdMaByLevelVid(sharmem_data_p->error_list_ag[cur_err_ar_idx].level,
            sharmem_data_p->error_list_ag[cur_err_ar_idx].vid, &error_md_p, &error_ma_p)
          &&error_md_p->name_length == md_p->name_length
          &&!memcmp(error_md_p->name_a, md_p->name_a, md_p->name_length))
        {
            /*remove first node*/
            if(cur_err_ar_idx==error_list_head->first_ar_idx)
            {
                error_list_head->first_ar_idx = sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
                pre_err_ar_idx=error_list_head->first_ar_idx;

                /*there are not node*/
                if(-1 == error_list_head->first_ar_idx)
                {
                    error_list_head->last_ar_idx= -1;
                }

                error_list_head->errors_number --;
                sharmem_data_p->error_list_ag[cur_err_ar_idx].used=FALSE;
                cur_err_ar_idx=pre_err_ar_idx;
                continue;
            }
            /*remove last node*/
            else if(cur_err_ar_idx == error_list_head->last_ar_idx)
            {
                 sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=-1;
                 error_list_head->last_ar_idx=pre_err_ar_idx;
            }
            /*remove middle node*/
            else
            {
                sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
            }
            error_list_head->errors_number --;
            sharmem_data_p->error_list_ag[cur_err_ar_idx].used=FALSE;
            cur_err_ar_idx=sharmem_data_p->error_list_ag[pre_err_ar_idx].nxt_ar_idx;
            continue;
        }
        pre_err_ar_idx=cur_err_ar_idx;
        cur_err_ar_idx=sharmem_data_p->error_list_ag[cur_err_ar_idx].nxt_ar_idx;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_RemoveErrorByDomainName*/


/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetErrorListHeadPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the error list head pointer
 * INPUT    : None
 * OUTPUT   : error list head pointer
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static CFM_OM_ErrorListHead_T * CFM_OM_LocalGetErrorListHeadPtr()
{
    return &sharmem_data_p->error_list_head_g;
}/*end of CFM_OM_LocalGetErrorListHeadPtr*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_ClearLoopBackList
 * ------------------------------------------------------------------------
 * PURPOSE  : This function clear the loopbacklist
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_ClearLoopBackList()
{
    CFM_OM_LoopBackListHead_T *loopback_list_head;

    CFM_OM_LOCK();
    loopback_list_head = CFM_OM_LocalGetLoopBackListHeadPtr();
    loopback_list_head->first_ar_idx=-1;
    loopback_list_head->last_ar_idx=-1;
    memset(sharmem_data_p->lbr_list_ag, 0, sizeof(sharmem_data_p->lbr_list_ag));
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_ClearLoopBackList*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddLoopBack
 * ------------------------------------------------------------------------
 * PURPOSE  : This function add the loopback
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE     - success
 *            FALSE    - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddLoopBack(UI32_T seq_num, UI32_T rcvd_time, CFM_OM_MEP_T *mep_p)
{
    CFM_OM_LoopBackListElement_T *loopback_p=NULL;
    I16_T ar_idx=-1;
    CFM_OM_LOCK();
    /* for old looback, may be removed in future
     */
    if(-1==(ar_idx=CFM_OM_GetFreeLoopBackAllocation()))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    loopback_p=&sharmem_data_p->lbr_list_ag[ar_idx];
    loopback_p->ar_idx=ar_idx;

    loopback_p->lbm_seq_number=seq_num;
    loopback_p->received_time=rcvd_time;
    loopback_p->md_ar_idx = mep_p->md_ar_idx;
    loopback_p->ma_ar_idx= mep_p->ma_ar_idx;
    loopback_p->used=TRUE;
    CFM_OM_AddToLoopBackList(loopback_p);
    CFM_OM_UNLOCK();
    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetSNMPCcStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get continuous check snmp trap status
 * INPUT    : trap         - the continuous check snmp trap type
 *            trap_enabled - the trap enable status
 * OUTPUT   : None
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetSNMPCcStatus(
                            CFM_TYPE_SnmpTrapsCC_T trap,
                            BOOL_T trap_enabled)
{
    CFM_OM_LOCK();
    switch(trap)
    {
        case CFM_TYPE_SNMP_TRAPS_CC_ALL:
            sharmem_data_p->global_cfg_g.snmp_cc_config_trap= trap_enabled;
            sharmem_data_p->global_cfg_g.snmp_cc_mep_up_trap = trap_enabled;
            sharmem_data_p->global_cfg_g.snmp_cc_mep_down_trap = trap_enabled;
            sharmem_data_p->global_cfg_g.snmp_cc_loop_trap = trap_enabled;
            break;

        case CFM_TYPE_SNMP_TRAPS_CC_MEP_UP:
            sharmem_data_p->global_cfg_g.snmp_cc_mep_up_trap=trap_enabled;
            break;

        case CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN:
            sharmem_data_p->global_cfg_g.snmp_cc_mep_down_trap=trap_enabled;
            break;

        case CFM_TYPE_SNMP_TRAPS_CC_CONFIG:
            sharmem_data_p->global_cfg_g.snmp_cc_config_trap=trap_enabled;
            break;

        case CFM_TYPE_SNMP_TRAPS_CC_LOOP:
            sharmem_data_p->global_cfg_g.snmp_cc_loop_trap=trap_enabled;
            break;

        default:
            CFM_OM_UNLOCK();
            return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetSNMPCcStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetSNMPCcStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the snmp continous check status
 * INPUT    : trap            - the cc trap type
 *            *trap_enabled_p - the trap enable status
 * OUTPUT   : None
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetSNMPCcStatus(
                            CFM_TYPE_SnmpTrapsCC_T trap,
                            BOOL_T *trap_enabled_p )
{
    CFM_OM_LOCK();
    switch(trap)
    {
        case CFM_TYPE_SNMP_TRAPS_CC_ALL:
            *trap_enabled_p = sharmem_data_p->global_cfg_g.snmp_cc_config_trap
                            &sharmem_data_p->global_cfg_g.snmp_cc_mep_up_trap
                            &sharmem_data_p->global_cfg_g.snmp_cc_mep_down_trap
                            &sharmem_data_p->global_cfg_g.snmp_cc_loop_trap;
            break;

        case CFM_TYPE_SNMP_TRAPS_CC_MEP_UP:
            *trap_enabled_p =sharmem_data_p->global_cfg_g.snmp_cc_mep_up_trap;
            break;

        case CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN:
            *trap_enabled_p =sharmem_data_p->global_cfg_g.snmp_cc_mep_down_trap;
            break;

        case CFM_TYPE_SNMP_TRAPS_CC_CONFIG:
            *trap_enabled_p =sharmem_data_p->global_cfg_g.snmp_cc_config_trap;
            break;

        case CFM_TYPE_SNMP_TRAPS_CC_LOOP:
            *trap_enabled_p =sharmem_data_p->global_cfg_g.snmp_cc_loop_trap;
            break;

        default:
            CFM_OM_UNLOCK();
            return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetSNMPCcStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetSNMPCrossCheckStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function set the snmp cross check status
 * INPUT    : trap   - the snmp cross check type
 *            tra_enable - the trap type enable status
 * OUTPUT   :
 * RETUEN   : TRUE     : success
 *            FALSE    : fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetSNMPCrossCheckStatus(
                                CFM_TYPE_SnmpTrapsCrossCheck_T trap,
                                BOOL_T trap_enabled)
{
    CFM_OM_LOCK();
    switch(trap)
    {
        case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL:
            sharmem_data_p->global_cfg_g.snmp_cross_check_ma_up_trap = trap_enabled;
            sharmem_data_p->global_cfg_g.snmp_cross_check_mep_missing_trap = trap_enabled;
            sharmem_data_p->global_cfg_g.snmp_cross_check_mep_unknown_trap = trap_enabled;
            break;

        case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP:
            sharmem_data_p->global_cfg_g.snmp_cross_check_ma_up_trap=trap_enabled;
            break;

        case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING:
            sharmem_data_p->global_cfg_g.snmp_cross_check_mep_missing_trap=trap_enabled;
            break;

        case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN:
            sharmem_data_p->global_cfg_g.snmp_cross_check_mep_unknown_trap=trap_enabled;
            break;

        default:
            CFM_OM_UNLOCK();
            return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_SetSNMPCrossCheckStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetSNMPCrossCheckStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the snmp cross check trap enabled status
 * INPUT    : trap            - the snamp trape type
 *            *trap_enabled_p - the trap type enable status
 * OUTPUT   :
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetSNMPCrossCheckStatus(
                                    CFM_TYPE_SnmpTrapsCrossCheck_T trap,
                                    BOOL_T *trap_enabled_p)
{
    CFM_OM_LOCK();
    switch(trap)
    {
        case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL:
            *trap_enabled_p=sharmem_data_p->global_cfg_g.snmp_cross_check_ma_up_trap
                           &sharmem_data_p->global_cfg_g.snmp_cross_check_mep_missing_trap
                           &sharmem_data_p->global_cfg_g.snmp_cross_check_mep_unknown_trap;
            break;

        case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP:
            *trap_enabled_p=sharmem_data_p->global_cfg_g.snmp_cross_check_ma_up_trap;
            break;

        case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING:
            *trap_enabled_p=sharmem_data_p->global_cfg_g.snmp_cross_check_mep_missing_trap;
            break;

        case CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN:
           *trap_enabled_p=sharmem_data_p->global_cfg_g.snmp_cross_check_mep_unknown_trap;
            break;

        default:
            CFM_OM_UNLOCK();
            return FALSE;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*end of CFM_OM_GetSNMPCrossCheckStatus */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the ma ais period
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *period_p - the ais period pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetAisPeriod(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T *period_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *period_p = ma_p->ais_period;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetAisPeriod*/

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  : This function get the next ma ais period
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *period_p - the ais period pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextAisPeriod(
                            UI32_T md_index,
                            UI32_T *ma_index_p,
                            UI32_T *period_p)
{
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(NULL == (ma_p = CFM_OM_LocalGetNextMaByMaIndex(md_index, *ma_index_p)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *ma_index_p =ma_p->index;
    *period_p = ma_p->ais_period;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetNextAisPeriod*/
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  :This function set the ais period
 * INPUT    : md_index  - the md index
 *                ma_index  - the ma index
 *                period      - the ais period
 * OUTPUT   :
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    :CFM_TYPE_AIS_PERIOD_1S, CFM_TYPE_AIS_PERIOD_60S
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetAisPeriod(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T period)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    ma_p->ais_period = period;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetAisPeriod*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the ais level
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :  *ais_level_p - the ais level pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetAisLevel(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_MdLevel_T *ais_level_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *ais_level_p= ma_p->ais_send_level;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetAisLevel*/

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the next ma ais level
 * INPUT    : md_index    - the md index
 *            ma_index_p  - the ma index
 * OUTPUT   : ais_level_p - the ais level pointer
 * RETUEN   : TRUE        - success
 *            FALSE       - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextAisLevel(
                            UI32_T md_index,
                            UI32_T *ma_index_p,
                            CFM_TYPE_MdLevel_T *ais_level_p)
{
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(NULL == (ma_p = CFM_OM_LocalGetNextMaByMaIndex(md_index, *ma_index_p)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *ma_index_p =ma_p->index;
    *ais_level_p = ma_p->ais_send_level;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetNextAisLevel*/
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :This function se the ma ais level
 * INPUT    : md_index  - the md index
 *            ma_index  - the ma index
 *            ais_level - the ais level
 * OUTPUT   :
 * RETUEN   : TRUE      - success
 *            FALSE     - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetAisLevel(
    UI32_T              md_index,
    UI32_T              ma_index,
    CFM_TYPE_MdLevel_T  ais_level)
{
    CFM_OM_MD_T *md_p = NULL;
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }
    ma_p->ais_send_level = ais_level;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetAisLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : md_index     - the md index
 *            ma_index     - the ma index
 * OUTPUT   : ais_status_p - the ais status pointer
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetAisStatus(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_AIS_STATUS_T *ais_status_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *ais_status_p = ma_p->ais_status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetAisStatus*/

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the next ma ais status
 * INPUT    : md_index     - the md index
 *            ma_index     - the ma index
 * OUTPUT   : ais_status_p - the ais status pointer
 * RETUEN   : TRUE         - success
 *            FALSE        - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextAisStatus(
                            UI32_T md_index,
                            UI32_T *ma_index_p,
                            CFM_TYPE_AIS_STATUS_T *ais_status_p)
{
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(NULL == (ma_p = CFM_OM_LocalGetNextMaByMaIndex(md_index, *ma_index_p)))
    {
        CFM_OM_LOCK();
        return FALSE;
    }

    *ma_index_p =ma_p->index;
    *ais_status_p = ma_p->ais_period;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetNextAisStatus*/
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function set the ais status
 * INPUT    : md_index   - the md index
 *            ma_index   - the ma index
 *            ais_status - the ais status
 * OUTPUT   :
 * RETUEN   : TRUE       - success
 *            FALSE      - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetAisStatus(
                        UI32_T md_index,
                        UI32_T ma_index,
                        CFM_TYPE_AIS_STATUS_T ais_status)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    ma_p->ais_status = ais_status;
    CFM_OM_UNLOCK();

    return TRUE;
}/*End of CFM_OM_SetAisStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetAisSupressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the AIS suppress status
 * INPUT    : md_index             - the md index
 *            ma_index             - the ma index
 * OUTPUT   : ais_supress_status_p - the ais suppress status pointer
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetAisSupressStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_AIS_STATUS_T *ais_supress_status_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *ais_supress_status_p = ma_p->ais_supress_status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetAisSupressStatus*/

#if 0 //not used at present
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the next ais suppress status
 * INPUT    : md_index             - the md index
 *            ma_index_p           - the ma index
 * OUTPUT   : ais_supress_status_p - the ais suppress status pointer
 * RETUEN   : TRUE                 - success
 *            FALSE                - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextAisSuppressStatus(
                                UI32_T md_index,
                                UI32_T *ma_index_p,
                                CFM_TYPE_AIS_STATUS_T *ais_supress_status_p)
{
    CFM_OM_MA_T *ma_p = NULL;

    CFM_OM_LOCK();
    if(NULL == (ma_p = CFM_OM_LocalGetNextMaByMaIndex(md_index, *ma_index_p)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *ma_index_p =ma_p->index;
    *ais_supress_status_p = ma_p->ais_supress_status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetNextAisSuppressStatus*/
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_SetAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function se the AIS suppress status
 * INPUT    : md_index           - the md index
 *            ma_index           - the ma index
 *            ais_supress_status - the ais suppress status
 * OUTPUT   : None
 * RETUEN   : TRUE            - success
 *            FALSE           - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T CFM_OM_SetAisSuppressStatus(
    UI32_T                  md_index,
    UI32_T                  ma_index,
    CFM_TYPE_AIS_STATUS_T   ais_supress_status)
{
    CFM_OM_MD_T *md_p =NULL;
    CFM_OM_MA_T *ma_p =NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    ma_p->ais_supress_status = ais_supress_status;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_SetAisStatus*/
#if 0
/** get info **/
#endif
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCfmGlobalCofigurationGlobalInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the cfm global configuration
 * INPUT    : None
 * OUTPUT   : *global_info_p
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCfmGlobalCofigurationGlobalInfo(
                            CFM_OM_GlobalConfigInfo_T *global_info_p)
{
    if(NULL == global_info_p)
        return FALSE;

    {
        CFM_TYPE_CfmStatus_T status;

        CFM_OM_GetCFMGlobalStatus(&status);
        global_info_p->cfm_global_status=status;
    }
    {
        /*cross check configuration*/
        UI32_T delay;

        CFM_OM_GetCrossCheckStartDelay(&delay);
        global_info_p->start_delay=delay;
    }
    {/*link trace cache hold time*/
        UI32_T linkTraceHoldTime;

        CFM_OM_GetLinkTraceCacheHoldTime(&linkTraceHoldTime);
        global_info_p->linktrace_cache_holdTime=linkTraceHoldTime/60; /*change to minutes*/
    }
    {  /*link trace cache size*/
        UI32_T linkTraceSize;

        CFM_OM_GetLinkTraceCacheSize(&linkTraceSize);
        global_info_p->linktrace_cache_size=linkTraceSize;
    }
    {/*link trace cache status*/
        CFM_TYPE_LinktraceStatus_T linkTraceCacheStatus;

        CFM_OM_GetLinkTraceCacheStatus(&linkTraceCacheStatus);
        global_info_p->linktrace_cache_status=linkTraceCacheStatus;
    }

    return TRUE;
}/*End of CFM_OM_GetCfmGlobalCofigurationGlobalInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCfmGlobalCofigurationTrapInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will get the trap configuration info
 * INPUT    : *trap_info  - the trap configuration info
 * OUTPUT   : trap_info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetCfmGlobalCofigurationTrapInfo(
                        CFM_OM_GlobalConfigInfo_T *global_info_p)
{
    BOOL_T trap_enabled=FALSE;

    if(NULL == global_info_p)
        return FALSE;

    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_MEP_UP, &trap_enabled);
    global_info_p->cc_mep_up=trap_enabled;

    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN, &trap_enabled);
    global_info_p->cc_mep_down=trap_enabled;

    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_CONFIG, &trap_enabled);
    global_info_p->cc_config=trap_enabled;

    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_LOOP, &trap_enabled);
    global_info_p->cc_loop=trap_enabled;

    CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN, &trap_enabled);
    global_info_p->cross_mep_unknown=trap_enabled;


    CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING,&trap_enabled);
    global_info_p->cross_mep_missing=trap_enabled;

    CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP,&trap_enabled);
    global_info_p->cross_ma_up=trap_enabled;

    return TRUE;
}/*End of CFM_OM_GetCfmGlobalCofigurationGlobalInfo*/

/*------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetStackEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the mep information
 * INPUT    : stack_ifindex - the md index
 *            vid          - the ma index
 *            level        - the mep id
 *            stack_direction - the egress identifier
 *          *mep_info_p - the mep info pointer to put the mep info
 * OUTPUT   : *mep_info_p - the mep info
 * RETURN   : None
 * NOTE     :  This function only for MIB
 *            because a MEP Only can be configured one direction, so it needn't take care direction
 *            move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetStackEntry(
                            UI32_T stack_lport,
                            UI16_T vid,
                            CFM_TYPE_MdLevel_T level,
                            CFM_TYPE_MP_Direction_T stack_direction,
                            CFM_OM_MepInfo_T *mep_info_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    if(NULL == mep_info_p)
        return FALSE;

    CFM_OM_LOCK();

    if(FALSE == CFM_OM_LocalGetMdMaByLevelVid(level, vid, &md_p, &ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    mep_p->md_index=md_p->index;
    mep_p->ma_index=ma_p->index;
    mep_p->lport=stack_lport;

    if(FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_LPORT_MD_MA_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillMepInfo(mep_p, mep_info_p);
    CFM_OM_UNLOCK();
    return TRUE;
}/*ENd of CFM_OM_GetStackEntry*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetNextStackEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep information
 * INPUT    : *stack_ifindex_p - the md index
 *            *vid_p          - the ma index
 *            *level_p     - the mep id
 *            *stack_direction_p - the egress identifier
 *            *mep_info_p - the mep info pointer to put the mep info
 * OUTPUT   : *stack_ifindex_p - the md index
 *            *vid_p          - the ma index
 *            *level_p     - the mep id
 *            *stack_direction_p - the egress identifier
 *            *mep_info_p - the mep info
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextStackEntry(
                            UI32_T *stack_lport_p,
                            UI16_T *vid_p,
                            CFM_TYPE_MdLevel_T *level_p,
                            CFM_TYPE_MP_Direction_T *stack_direction_p,
                            CFM_OM_MepInfo_T *return_mep_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p=NULL;
    CFM_OM_MEP_T *mep_p=&mep_tmp2_g, *next_mep_p=NULL;
    UI32_T nxt_md_index=0, nxt_ma_index=0, nxt_lport;
    UI16_T nxt_level=0xffff, nxt_vid=0xffff;
    BOOL_T find=FALSE;

    if(NULL == stack_lport_p
        || NULL == vid_p
        || NULL == level_p
        || NULL == stack_direction_p
        || NULL == return_mep_p)
        return FALSE;

    nxt_lport=*stack_lport_p;
    mep_p->md_index=nxt_md_index;
    mep_p->ma_index=nxt_ma_index;
    mep_p->lport=nxt_lport;

    CFM_OM_LOCK();

    while(TRUE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_LPORT_MD_MA_KEY))
    {
        if(mep_p->lport > nxt_lport)
        {
            next_mep_p=mep_p;
            find=TRUE;
            break;
        }

        if(sharmem_data_p->md_ag[mep_p->md_ar_idx].level == *level_p)
        {
            if((mep_p->primary_vid > *vid_p)&&( (mep_p->primary_vid -*vid_p)<(nxt_vid-*vid_p)))
            {
                nxt_vid=mep_p->primary_vid;
                nxt_level=sharmem_data_p->md_ag[mep_p->md_ar_idx].level;
                find=TRUE;
            }
        }
        else if( (sharmem_data_p->md_ag[mep_p->md_ar_idx].level - *level_p)<(nxt_level-*level_p))
        {
            nxt_vid=mep_p->primary_vid;
            nxt_level=sharmem_data_p->md_ag[mep_p->md_ar_idx].level;
            find=TRUE;
        }
    }

    if(TRUE == find)
    {
        if(NULL==next_mep_p)
        {
            if(FALSE == CFM_OM_LocalGetMdMaByLevelVid(nxt_level, nxt_vid, &md_p, &ma_p))
            {
                CFM_OM_UNLOCK();
                return FALSE;
            }

            mep_p->md_index=md_p->index;
            mep_p->ma_index=ma_p->index;
            mep_p->lport=*stack_lport_p;
            if(FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_LPORT_MD_MA_KEY ))
            {
                CFM_OM_UNLOCK();
                return FALSE;
            }

            next_mep_p = mep_p;
        }

        CFM_OM_LocalFillMepInfo(next_mep_p, return_mep_p);

        *stack_lport_p=next_mep_p->lport;
        *level_p=sharmem_data_p->md_ag[next_mep_p->md_ar_idx].level;
        *stack_direction_p=next_mep_p->direction;
        *vid_p=next_mep_p->primary_vid;

        CFM_OM_UNLOCK();
        return TRUE;
    }

    CFM_OM_UNLOCK();
    return FALSE ;
}/*End of CFM_OM_GetNextStackEntry*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetErrorInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the configure error entry
 * INPUT    : vid           - the vlan id
 *            lport         - the ifindex
 *            *error_info_p - the error info pointer to put the error info
 * OUTPUT   : *error_info_p - the error info
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetErrorInfo(
                        UI16_T vid,
                        UI32_T lport,
                        CFM_OM_Error_T *error_info_p)
{
    CFM_OM_ErrorListElement_T *nxt_queue_element=NULL;

    if(NULL == error_info_p)
        return FALSE;

    CFM_OM_LOCK();
    if(TRUE == CFM_OM_LocalGetError(vid, lport, &nxt_queue_element))
    {
       CFM_OM_LocalFillErrorInfo(nxt_queue_element, error_info_p);
       CFM_OM_UNLOCK();
       return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*End of CFM_OM_GetErrorInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextErrorInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the Next error info
 * INPUT    : *vid_p        - the vlan id
 *            *lport_p      - the logical port
 *¡@¡@¡@¡@¡@   *error_info_p - the error_p info pointer
 * OUTPUT   : *error_info_p - the error element in list
 * RETURN   : TRUE     - success
 *            FALSE    - fail
 * NOTE     :move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextErrorInfo(
                        UI16_T *vid_p,
                        UI32_T *lport_p,
                        CFM_OM_Error_T *error_info_p)
{
    CFM_OM_ErrorListElement_T *nxt_queue_element=NULL;

    if(NULL == error_info_p
       || NULL == vid_p
       || NULL == lport_p)
    {
        return FALSE;
    }

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetNextError(*vid_p, *lport_p, &nxt_queue_element))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *vid_p=nxt_queue_element->vid;
    *lport_p=nxt_queue_element->lport;

    CFM_OM_LocalFillErrorInfo(nxt_queue_element, error_info_p);
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetNextErrorInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextFngConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the fault notify configure
 * INPUT    : *md_index    - the md index pointer
 *            *ma_index    - the ma index pointer
 *            *mep_id      - the mep id pointer
 * OUTPUT   : *fng_config  - the fng configure pointer
 *            *md_index    - the next md index pointer
 *            *ma_index    - the next ma index pointer
 *            *mep_id      - the nextx mep id pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : move to om
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextFngConfig(
    UI32_T                  *md_index_p,
    UI32_T                  *ma_index_p,
    UI32_T                  *mep_id_p,
    CFM_OM_MepFngConfig_T   *fng_config_p)
{
    CFM_OM_MEP_T *mep_p = &mep_tmp3_g;

    if((NULL==md_index_p )||(NULL == ma_index_p)||(NULL == mep_id_p) ||(NULL == fng_config_p))
    {
        return FALSE;
    }

    memset(fng_config_p, 0, sizeof(CFM_OM_MepFngConfig_T));

    mep_p->md_index=*md_index_p;
    mep_p->ma_index=*ma_index_p;
    mep_p->identifier=*mep_id_p;

    CFM_OM_LOCK();
    while (TRUE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        memcpy(fng_config_p->md_name_a, sharmem_data_p->md_ag[mep_p->md_ar_idx].name_a, sharmem_data_p->md_ag[mep_p->md_ar_idx].name_length);
        memcpy(fng_config_p->ma_name_a, sharmem_data_p->ma_ag[mep_p->ma_ar_idx].name_a, sharmem_data_p->ma_ag[mep_p->ma_ar_idx].name_length);
        fng_config_p->md_name_len = sharmem_data_p->md_ag[mep_p->md_ar_idx].name_length;
        fng_config_p->ma_name_len = sharmem_data_p->ma_ag[mep_p->ma_ar_idx].name_length;
        fng_config_p->mep_id           = mep_p->identifier;
        fng_config_p->fng_alarm_time   = mep_p->fng_alarm_time;
        fng_config_p->fng_reset_time   = mep_p->fng_reset_time;
        fng_config_p->highest_defect   = mep_p->highest_pri_defect;
        fng_config_p->lowest_alarm_pri = mep_p->lowest_priority_defect;
        *md_index_p=sharmem_data_p->md_ag[mep_p->md_ar_idx].index;
        *ma_index_p=sharmem_data_p->ma_ag[mep_p->ma_ar_idx].index;
        *mep_id_p=mep_p->identifier;
        CFM_OM_UNLOCK();
        return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*End of CFM_OM_GetNextFngConfig*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdMaIndexByMaName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the md ma index by ma name
 * INPUT    : *ma_name_ap   - the ma name array pointer
 *            name_len      - the ma name length
 * OUTPUT   : *md_index_p   - the md index pointer
 *            *ma_index_p   - the ma index pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdMaIndexByMaName(
                            UI8_T   *ma_name_ap,
                            UI32_T  name_len,
                            UI32_T  *md_index_p,
                            UI32_T  *ma_index_p)
{
    CFM_OM_MD_T *return_md_p;
    CFM_OM_MA_T *return_ma_p;
    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMdMaByMaName(ma_name_ap,  name_len, &return_md_p, &return_ma_p))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *md_index_p=return_md_p->index;
    *ma_index_p=return_ma_p->index;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetMdMaIndexByMaName*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdIndexByName
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the md index by md name
 * INPUT    : *md_name_ap   - the md name array pointer
 *            name_len      - the md name length
 * OUTPUT   : *md_index_p   - the md index pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdIndexByName(
                            UI8_T *md_name_ap,
                            UI32_T name_len,
                            UI32_T *md_index_p )
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_LOCK();
    md_p = CFM_OM_LocalGetMdByName(md_name_ap, name_len);

    if(NULL == md_p)
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    *md_index_p=md_p->index;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetMdIndexByName*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMdIndexByIndexAndLevel
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next md index by md index and level
 * INPUT    : level         - the md level
 * OUTPUT   : *md_index_p   - the md index pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : the next md is not the same level as the input md index's level
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMdIndexByIndexAndLevel(
                            UI8_T level,
                            UI32_T *md_index_p)
{
    CFM_OM_MD_T *md_p=NULL;
    BOOL_T      ret =FALSE;

    CFM_OM_LOCK();

    md_p = CFM_OM_LocalGetNextMdByLevel(*md_index_p, level);

    if (NULL != md_p)
    {
        *md_index_p = md_p->index;
        ret = TRUE;
    }
    CFM_OM_UNLOCK();

    return ret;
}/* End of CFM_OM_GetNextMdIndexByIndexAndLevel*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextLinktraceReplyInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next link trace reply
 * INPUT    : *md_index_p        - the md index poiner
 *            *ma_index_p        - the ma index pointer
 *            *mep_id_p          - the mep id pointer
 * OUTPUT   : *md_index_p        - the next md index poiner
 *            *ma_index_p        - the next ma index pointer
 *            *mep_id_p          - the next mep id pointer
 *            *linktrace_reply_p - the linktrace raply pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextLinktraceReplyInfo(
                                UI32_T *md_index_p,
                                UI32_T *ma_index_p,
                                UI32_T *mep_id_p,
                                UI32_T *seq_num_p,
                                UI32_T *rcvd_order_p,
                                CFM_OM_LinktraceReply_T *linktrace_reply_p)
{
    CFM_OM_LTR_T *retrieved_ltr_p=&ltr_tmp_g;

    retrieved_ltr_p->md_index=*md_index_p;
    retrieved_ltr_p->ma_index=*ma_index_p;
    retrieved_ltr_p->rcvd_mep_id=*mep_id_p;
    retrieved_ltr_p->seq_number=*seq_num_p;
    retrieved_ltr_p->receive_order=*rcvd_order_p;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetNextLtrRecord(retrieved_ltr_p, CFM_OM_MD_MA_MEP_SEQ_REC_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillLinktraceReplyInfo(retrieved_ltr_p, linktrace_reply_p);

    *md_index_p  =retrieved_ltr_p->md_index;
    *ma_index_p  =retrieved_ltr_p->ma_index;
    *mep_id_p    =retrieved_ltr_p->rcvd_mep_id;
    *seq_num_p   =retrieved_ltr_p->seq_number;
    *rcvd_order_p=retrieved_ltr_p->receive_order;

    CFM_OM_UNLOCK();
    return TRUE;
}/* End of CFM_OM_GetNextLinktraceReplyInfo*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLinktraceReplyInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next link trace reply
 * INPUT    : md_index        - the md index
 *           ma_index        - the ma index
 *           mep_id          - the mep id
 *           seq_num         - the sequence number
 *           rcvd_order      - the recevd order
 * OUTPUT   : *linktrace_reply_p - the linktrace raply pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetLinktraceReplyInfo(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T mep_id,
                                UI32_T seq_num,
                                UI32_T rcvd_order,
                                CFM_OM_LinktraceReply_T *linktrace_reply_p)
{
    CFM_OM_LTR_T *retrieved_ltr_p=&ltr_tmp_g;

    retrieved_ltr_p->md_index = md_index;
    retrieved_ltr_p->ma_index = ma_index;
    retrieved_ltr_p->rcvd_mep_id = mep_id;
    retrieved_ltr_p->receive_order = rcvd_order;
    retrieved_ltr_p->seq_number =seq_num;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetLtrExactRecord(retrieved_ltr_p, CFM_OM_MD_MA_MEP_SEQ_REC_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillLinktraceReplyInfo(retrieved_ltr_p, linktrace_reply_p);

    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetLinktraceReplyInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextLoopBackInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the Next loop back info
 * INPUT    : *loop_back_info_p.index  - the loop back info index
 * OUTPUT   : *loop_back_info_p        - the loop back info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextLoopBackInfo(
                                    CFM_OM_LoopbackInfo_T *loop_back_info_p)
{
    CFM_OM_LoopBackListElement_T *nxt_queue_element=NULL;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetNextLoopBackElement(loop_back_info_p->lbm_seq_num, &nxt_queue_element))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    loop_back_info_p->receivedTime=nxt_queue_element->received_time;
    loop_back_info_p->lbm_seq_num= nxt_queue_element->lbm_seq_number;

    memcpy(loop_back_info_p->md_name_a, sharmem_data_p->md_ag[nxt_queue_element->md_ar_idx].name_a,
            sharmem_data_p->md_ag[nxt_queue_element->md_ar_idx].name_length);
    loop_back_info_p->md_name_len = sharmem_data_p->md_ag[nxt_queue_element->md_ar_idx].name_length;
    memcpy(loop_back_info_p->ma_name_a, sharmem_data_p->ma_ag[nxt_queue_element->ma_ar_idx].name_a,
            sharmem_data_p->ma_ag[nxt_queue_element->ma_ar_idx].name_length);
    loop_back_info_p->ma_name_len= sharmem_data_p->ma_ag[nxt_queue_element->ma_ar_idx].name_length;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetNextLoopBackInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepErrorCCMLastFailure
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get error ccm content
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *content_p   - the failure array
 *          *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepErrorCCMLastFailure(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T *content_p,
                            UI32_T *content_len_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    memcpy(content_p, mep_p->error_ccm_last_failure_a, mep_p->error_ccm_last_failure_lenth);
    *content_len_p=mep_p->error_ccm_last_failure_lenth;

    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMepErrorCCMLastFailure
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get next error ccm content
 * INPUT    : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p   - the failure array
 *            *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMepErrorCCMLastFailure(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            UI8_T *content_p,
                            UI32_T *content_len_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=*md_index_p;
    mep_p->ma_index=*ma_index_p;
    mep_p->identifier=*mep_id_p;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    memcpy(content_p, mep_p->error_ccm_last_failure_a, mep_p->error_ccm_last_failure_lenth);
    *content_len_p=mep_p->error_ccm_last_failure_lenth;

    *md_index_p=sharmem_data_p->md_ag[mep_p->md_ar_idx].index;
    *ma_index_p=sharmem_data_p->ma_ag[mep_p->ma_ar_idx].index;
    *mep_id_p=mep_p->identifier;
    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepXconCcmLastFailure
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get xcon ccm content
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *content_p   - the failure array
 *          *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepXconCcmLastFailure(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T *content_p,
                            UI32_T *content_len_p)
{
    CFM_OM_MEP_T *mp_p=&mep_tmp3_g;
    mp_p->md_index=md_index;
    mp_p->ma_index=ma_index;
    mp_p->identifier=mep_id;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMepExactRecord(mp_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    memcpy(content_p, mp_p->xcon_ccm_last_failure_a, mp_p->xcon_ccm_last_failure_length);
    *content_len_p=mp_p->xcon_ccm_last_failure_length;
    CFM_OM_UNLOCK();
    return TRUE;
}/* End of CFM_OM_GetMepXconCcmLastFailure*/

#if 0 //not used at present
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepNextXconCcmLastFailure
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get next xcon  ccm content
 * INPUT    : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p   - the failure array
 *            *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepNextXconCcmLastFailure(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            UI8_T *content_p,
                            UI32_T *content_len_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=*md_index_p;
    mep_p->ma_index=*ma_index_p;
    mep_p->identifier=*mep_id_p;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    memcpy(content_p, mep_p->xcon_ccm_last_failure_a, mep_p->xcon_ccm_last_failure_length);
    *content_len_p=mep_p->xcon_ccm_last_failure_length;

    *md_index_p=sharmem_data_p->md_ag[mep_p->md_ar_idx].index;
    *ma_index_p=sharmem_data_p->ma_ag[mep_p->ma_ar_idx].index;
    *mep_id_p=mep_p->identifier;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetMepNextXconCcmLastFailure*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepTransmitLbmDataTlv
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get data tlv content
 * INPUT    : md_index - the md index
 *            ma_index - the ma index
 *            mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *content_p   - the failure array
 *          *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepTransmitLbmDataTlv(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T mep_id,
                            UI8_T *content_p,
                            UI32_T *content_len_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    /*now this data tlv value can not be modified by user, it will always 0xff*/
    {
        UI32_T i=0;

        for(i=0;i<mep_p->transmit_lbm_data_tlv_length; i++)
            content_p[i]=0xff;

        *content_len_p=mep_p->transmit_lbm_data_tlv_length;
    }
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetMepTransmitLbmDataTlv*/

#if 0 //not used at present
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepNextTransmitLbmDataTlv
 *-------------------------------------------------------------------------
 * PURPOSE  :This function will get next data  tlv content
 * INPUT    : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p - the buffer
 * OUTPUT   : *md_index - the md index
 *            *ma_index - the ma index
 *            *mep_id     - the mep identifier
 *            *content_p   - the failure array
 *            *content_len_p- the array length
 * RETURN   : TRUE - sucess
 *          FALSE - failure
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepNextTransmitLbmDataTlv(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            UI8_T *content_p,
                            UI32_T *content_len_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    mep_p->md_index=*md_index_p;
    mep_p->ma_index=*ma_index_p;
    mep_p->identifier=*mep_id_p;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    memcpy(content_p, mep_p->transmit_lbm_data_tlv_a, mep_p->transmit_lbm_data_tlv_length);
    *content_len_p=mep_p->transmit_lbm_data_tlv_length;

    *md_index_p=mep_p->md_p->index;
    *ma_index_p=mep_p->ma_p->index;
    *mep_id_p=mep_p->identifier;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetMepNextTransmitLbmDataTlv*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function Get the Md info
 * INPUT    : md_inde     - the md index
 * OUTPUT   : return_md_p - the md pointer from mgr.
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMdInfo(
                        UI32_T md_index,
                        CFM_OM_MdInfo_T *return_md_p)
{
    CFM_OM_MD_T *md_p=NULL;

    if(NULL == return_md_p)
        return FALSE;

    CFM_OM_LOCK();

    if(NULL == (md_p=CFM_OM_LocalGetMdByIndex(md_index)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillMdInfo(md_p, return_md_p);
    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function Get the next Md info
 * INPUT    : md_index_p  - the md index to get next md index
 * OUTPUT   : md_index_p  - the current md index
 *            return_md_p - the md pointer from mgr.
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMdInfo(
                        UI32_T *md_index_p,
                        CFM_OM_MdInfo_T *return_md_p)
{
    CFM_OM_MD_T *md_p=NULL;

    if(NULL == md_index_p
        || NULL == return_md_p
        )
        return FALSE;
    CFM_OM_LOCK();
    if(NULL == (md_p=CFM_OM_LocalGetNextMdByIndex(*md_index_p)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillMdInfo(md_p, return_md_p);
    *md_index_p = md_p->index;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetNextMdInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMaInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : this function get the ma info
 * INPUT    : md_index     - the md index
 *            ma_index     - the ma index
 * OUTPUT   : *return_ma_p - the return ma pointer from mgr
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMaInfo(
                    UI32_T md_index,
                    UI32_T ma_index,
                    CFM_OM_MaInfo_T *return_ma_p)
{
    CFM_OM_MA_T *ma_p=NULL;

    if(NULL == return_ma_p)
        return FALSE;
    CFM_OM_LOCK();
    if(NULL == (ma_p=CFM_OM_LocalGetMa(md_index, ma_index)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillMaInfo(ma_p, return_ma_p);
    CFM_OM_UNLOCK();
    return TRUE;
}/* End of CFM_OM_GetMaInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMaInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next Ma info
 * INPUT    : md_index      - the md index
 *            *ma_index      - the ma index
 * OUTPUT   : *ma_index the current ma index
 *          *return_ma_p  - the return ma info pointer from mgr
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMaInfo(
                    UI32_T md_index,
                    UI32_T *ma_index_p,
                    CFM_OM_MaInfo_T *return_ma_p)
{
    CFM_OM_MA_T *ma_p=NULL;

    if(NULL == return_ma_p
        || NULL == ma_index_p)
        return FALSE;
    CFM_OM_LOCK();
    if(NULL == (ma_p=CFM_OM_LocalGetNextMaByMaIndex(md_index,*ma_index_p)))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillMaInfo(ma_p, return_ma_p);

    *ma_index_p = ma_p->index;
    CFM_OM_UNLOCK();
    return TRUE;
}/* End of CFM_OM_GetNextMaInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : md_index    - the md index
 *           ma_index    - the ma index
 *           mep_id      - the mep id
 * OUTPUT   : *return_mep_p  - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if lport != 0, it means you want to get the mep specify on this port
 *           so, when get next mep is not on this port, it will return false
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepInfo(
                    UI32_T md_index,
                    UI32_T ma_index,
                    UI32_T mep_id,
                    CFM_OM_MepInfo_T *return_mep_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    if(NULL == return_mep_p)
        return FALSE;
    mep_p->md_index=md_index;
    mep_p->ma_index=ma_index;
    mep_p->identifier=mep_id;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetMepExactRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillMepInfo(mep_p, return_mep_p);
    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *mep_id_p      - the mep id pointer
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *mep_id_p      - the next mep id pointer
 *            *return_mep_p  - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : 1. if lport != 0, it means you want to get the mep specify on this port
 *               so, when get next mep is not on this port, it will return false
 *
 *            2. for CLI/WEB, mep with lport == 0 "CAN NOT BE" got from this api.
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMepInfo(
                        UI32_T *md_index_p,
                        UI32_T *ma_index_p,
                        UI32_T *mep_id_p,
                        UI32_T lport,
                        CFM_OM_MepInfo_T *return_mep_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    if(NULL == return_mep_p
        ||NULL == ma_index_p
        ||NULL == md_index_p
        || NULL == mep_id_p)
        return FALSE;

    mep_p->md_index=*md_index_p;
    mep_p->ma_index=*ma_index_p;
    mep_p->identifier=*mep_id_p;
    mep_p->lport = lport;

    CFM_OM_LOCK();

    do
    {
        if(lport == 0)
        {
            if(FALSE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
            {
                CFM_OM_UNLOCK();
                return FALSE;
            }
        }
        else
        {
            if(FALSE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_LPORT_MD_MA_KEY))
            {
                CFM_OM_UNLOCK();
                return FALSE;
            }

            if(mep_p->lport != lport)
            {
                CFM_OM_UNLOCK();
                return FALSE;
            }
        }
    } while (mep_p->lport == 0);

    CFM_OM_LocalFillMepInfo(mep_p, return_mep_p);

    *md_index_p=mep_p->md_index;
    *ma_index_p=mep_p->ma_index;
    *mep_id_p=mep_p->identifier;
    CFM_OM_UNLOCK();
    return TRUE;
}/* End of CFM_OM_GetNextMepInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetDot1agNextMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *mep_id_p      - the mep id pointer
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *mep_id_p      - the next mep id pointer
 *            *return_mep_p  - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : 1. if lport != 0, it means you want to get the mep specify on this port
 *               so, when get next mep is not on this port, it will return false
 *
 *            2. for SNMP, mep with lport == 0 "CAN BE" got from this api.
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetDot1agNextMepInfo(
                        UI32_T *md_index_p,
                        UI32_T *ma_index_p,
                        UI32_T *mep_id_p,
                        UI32_T lport,
                        CFM_OM_MepInfo_T *return_mep_p)
{
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    if(NULL == return_mep_p
        ||NULL == ma_index_p
        ||NULL == md_index_p
        || NULL == mep_id_p)
        return FALSE;

    mep_p->md_index=*md_index_p;
    mep_p->ma_index=*ma_index_p;
    mep_p->identifier=*mep_id_p;
    mep_p->lport = lport;
    CFM_OM_LOCK();

    if(lport == 0)
    {
        if(FALSE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_MD_MA_MEP_KEY))
        {
            CFM_OM_UNLOCK();
            return FALSE;
        }
    }
    else
    {
        if(FALSE == CFM_OM_LocalGetNextMepRecord(mep_p, CFM_OM_LPORT_MD_MA_KEY))
        {
            CFM_OM_UNLOCK();
            return FALSE;
        }

        if(mep_p->lport != lport)
        {
            CFM_OM_UNLOCK();
            return FALSE;
        }
    }

    CFM_OM_LocalFillMepInfo(mep_p, return_mep_p);

    *md_index_p=mep_p->md_index;
    *ma_index_p=mep_p->ma_index;
    *mep_id_p=mep_p->identifier;
    CFM_OM_UNLOCK();
    return TRUE;
}/* End of CFM_OM_GetDot1agNextMepInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextMipInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *lport               - the logical port
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *lport               - the logical port
 *            *return_mip_p  - the mep info pointer
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : if lport != 0, it means you want to get the mep specify on this port
 *           so, when get next mep is not on this port, it will return false
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMipInfo(
                    UI32_T *md_index_p,
                    UI32_T *ma_index_p,
                    UI32_T *lport_p,
                    CFM_OM_MipInfo_T *return_mip_p)
{
    CFM_OM_MIP_T mip;

    if(NULL == md_index_p
        || NULL == ma_index_p
        || NULL == lport_p
        || NULL == return_mip_p)
        return FALSE;

    mip.md_index=*md_index_p;
    mip.ma_index=*ma_index_p;
    mip.lport=*lport_p;

    CFM_OM_LOCK();
    if(FALSE == CFM_OM_LocalGetNextMipRecord(&mip, CFM_OM_LPORT_MD_MA_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    memset(return_mip_p, 0, sizeof(CFM_OM_MipInfo_T));

    *md_index_p = mip.md_index;
    *ma_index_p = mip.ma_index;
    *lport_p    = mip.lport;

    return_mip_p->md_index = sharmem_data_p->md_ag[mip.md_ar_idx].index;
    memcpy(return_mip_p->md_name_a, sharmem_data_p->md_ag[mip.md_ar_idx].name_a, sharmem_data_p->md_ag[mip.md_ar_idx].name_length);
    return_mip_p->md_name_len = sharmem_data_p->md_ag[mip.md_ar_idx].name_length;
    return_mip_p->md_level = sharmem_data_p->md_ag[mip.md_ar_idx].level;

    return_mip_p->ma_index=sharmem_data_p->ma_ag[mip.ma_ar_idx].index;
    memcpy(return_mip_p->ma_name_a, sharmem_data_p->ma_ag[mip.ma_ar_idx].name_a, sharmem_data_p->ma_ag[mip.ma_ar_idx].name_length);
    return_mip_p->ma_name_len = sharmem_data_p->ma_ag[mip.ma_ar_idx].name_length;
    return_mip_p->vid = sharmem_data_p->ma_ag[mip.ma_ar_idx].primary_vid;
    return_mip_p->lport=mip.lport;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetNextMipInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRemoteMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the next cross check remote mep info
 * INPUT    : *md_index_p    - the md index poiner
 *            *ma_index_p    - the ma index pointer
 *            *mep_id_p      - the mep id pointer
 *            current_time  - current_time
 * OUTPUT   : *md_index_p    - the next md index poiner
 *            *ma_index_p    - the next ma index pointer
 *            *mep_id_p      - the next mep id pointer
 *            *remote_mep_p  - the remote mep info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextRemoteMepInfo(
                            UI32_T *md_index_p,
                            UI32_T *ma_index_p,
                            UI32_T *mep_id_p,
                            UI32_T current_time,
                            CFM_OM_RemoteMepCrossCheck_T *return_rmep_p)
{
    CFM_OM_REMOTE_MEP_T rmep;

    if((NULL == md_index_p)
        ||(NULL == ma_index_p)
        ||(NULL == mep_id_p)
        ||(NULL == return_rmep_p))
    {
        return FALSE;
    }

    rmep.md_index=*md_index_p;
    rmep.ma_index=*ma_index_p;
    rmep.identifier=*mep_id_p;

    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetNextRemoteMepRecord(&rmep, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillRemoteMepInfo(&rmep, return_rmep_p, current_time);

    *md_index_p=rmep.md_index;
    *ma_index_p=rmep.ma_index;
    *mep_id_p  =rmep.identifier;
    CFM_OM_UNLOCK();
    return TRUE;
}/* End of CFM_OM_GetNextRemoteMepInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRemoteMepInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the cross check remote mep info
 * INPUT    : md_index    - the md index
 *           ma_index    - the ma index
 *           remote_mep_id     - the remtoe mep id
 *           current_tiem      - current time
 * OUTPUT   : *remote_mep_p  - the remote mep info
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetRemoteMepInfo(
                            UI32_T md_index,
                            UI32_T ma_index,
                            UI32_T remote_mep_id,
                            UI32_T current_time,
                            CFM_OM_RemoteMepCrossCheck_T *return_rmep_p)
{
    CFM_OM_REMOTE_MEP_T rmep;

    if(NULL == return_rmep_p)
    {
        return FALSE;
    }
    CFM_OM_LOCK();
    rmep.md_index=md_index;
    rmep.ma_index=ma_index;
    rmep.identifier=remote_mep_id;

    if (FALSE == CFM_OM_LocalGetRemoteMepExactRecord(&rmep, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillRemoteMepInfo(&rmep, return_rmep_p, current_time);
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetRemoteMepInfo*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetMepDbTable
 *-------------------------------------------------------------------------
 * PURPOSE  :This function get mep info
 * INPUT    : md_index      - the md index
 *            ma_index      - the ma index
 *            mep_id        - the mep id
 *            remote_mep_id - the egress identifier
 * OUTPUT   : return_rmep_p - the remtoe mep info
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetMepDbTable(
                        UI32_T md_index,
                        UI32_T ma_index,
                        UI32_T mep_id,
                        UI32_T remote_mep_id,
                        UI32_T current_time,
                        CFM_OM_RemoteMepCrossCheck_T *return_rmep_p)
{
    CFM_OM_REMOTE_MEP_T rmep;

    if(NULL == return_rmep_p)
        return FALSE;

    rmep.md_index=md_index;
    rmep.ma_index=ma_index;
    rmep.identifier=remote_mep_id;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetRemoteMepExactRecord(&rmep, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    if(mep_id==rmep.rcvd_mep_id)
    {
        CFM_OM_LocalFillRemoteMepInfo(&rmep, return_rmep_p, current_time);
        CFM_OM_UNLOCK();
        return TRUE;
    }
    CFM_OM_UNLOCK();
    return FALSE;
}/*End of CFM_OM_GetMepDbTable*/

/*-------------------------------------------------------------------------
 *FUNCTION NAME - CFM_OM_GetNextMepDbTable
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get next mep info
 * INPUT    : md_index_p      - the md index
 *            ma_index_p      - the ma index
 *            remote_mep_id_p - the remote mep identifier
 * OUTPUT   : md_index_p      - the next md index
 *            ma_index_p      - the next ma index
 *            mep_id_p        - the next mep id
 *            remote_mep_id_p - the next remote mep id
 *            return_rmep_p   - the remtoe mep info
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextMepDbTable(
                        UI32_T *md_index_p,
                        UI32_T *ma_index_p,
                        UI32_T *mep_id_p,
                        UI32_T *remote_mep_id_p,
                        UI32_T current_time,
                        CFM_OM_RemoteMepCrossCheck_T *return_rmep_p)
{
    CFM_OM_REMOTE_MEP_T rmep;

    if(NULL == return_rmep_p
        || NULL == md_index_p
        || NULL == ma_index_p
        || NULL == mep_id_p
        || NULL == remote_mep_id_p
        )
        return FALSE;

    rmep.md_index=*md_index_p;
    rmep.ma_index=*ma_index_p;
    rmep.identifier=*remote_mep_id_p;
    CFM_OM_LOCK();
    if (FALSE == CFM_OM_LocalGetNextRemoteMepRecord(&rmep, CFM_OM_MD_MA_MEP_KEY))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    CFM_OM_LocalFillRemoteMepInfo(&rmep, return_rmep_p, current_time);

    *md_index_p=rmep.md_index;
    *ma_index_p=rmep.ma_index;
    *remote_mep_id_p  =rmep.identifier;
    *mep_id_p=rmep.rcvd_mep_id;
    CFM_OM_UNLOCK();
    return TRUE;
}/*End of CFM_OM_GetMepDbTable*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetDelayMeasureResult
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the delay measure result.
 * INPUT    : None
 * OUTPUT   : avg_fd_ms_p  - pointer to content of avarge frame delay in ms
 *            min_fd_ms_p  - pointer to content of minimum frame delay in ms
 *            max_fd_ms_p  - pointer to content of maximum frame delay in ms
 *            avg_fdv_ms_p - pointer to content of
 *                             avarage frame delay variation in ms
 *            succ_cnt_p   - pointer to content of received dmr's count
 *            total_cnt_p  - pointer to content of sent dmm's count
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetDelayMeasureResult(
                    UI32_T  *avg_fd_ms_p,
                    UI32_T  *min_fd_ms_p,
                    UI32_T  *max_fd_ms_p,
                    UI32_T  *avg_fdv_ms_p,
                    UI32_T  *succ_cnt_p,
                    UI32_T  *total_cnt_p)
{
    CFM_OM_DmmCtrlRec_T     *dmm_ctrl_rec_p;
    CFM_OM_DmrRec_T         *dmr_rec_p;
    UI32_T                  idx,
                            min_fd_ms=0xffffffff,  /* none can be bigger  than this */
                            max_fd_ms=0,           /* none can be smaller than this */
                            avg_fd_ms=0, succ_cnt=0, total_fd_ms =0;
    UI32_T                  avg_fdv_ms =0, total_fdv_ms =0;
    BOOL_T                  ret = FALSE;

    if(NULL == avg_fd_ms_p
        || NULL == min_fd_ms_p
        || NULL == max_fd_ms_p
        || NULL == succ_cnt_p
        || NULL == total_cnt_p
        )
        return FALSE;

    CFM_OM_LOCK();
    dmm_ctrl_rec_p = CFM_OM_GetDmmCtrlRecPtr();

    if (NULL != dmm_ctrl_rec_p)
    {
        if (dmm_ctrl_rec_p->cur_dmm_seq == dmm_ctrl_rec_p->cur_rcv_idx)
        {
            for (idx =0; idx < dmm_ctrl_rec_p->counts; idx++)
            {
                dmr_rec_p = &dmm_ctrl_rec_p->dmr_rec_ar[idx];

                if (dmr_rec_p->rec_state == CFM_TYPE_DMM_REPLY_REC_STATE_RECEIVED)
                {
                    succ_cnt    += 1;
                    total_fd_ms += dmr_rec_p->frame_delay_ms;

                    if (min_fd_ms > dmr_rec_p->frame_delay_ms)
                    {
                        min_fd_ms = dmr_rec_p->frame_delay_ms;
                    }

                    if (max_fd_ms < dmr_rec_p->frame_delay_ms)
                    {
                        max_fd_ms = dmr_rec_p->frame_delay_ms;
                    }

                    total_fdv_ms += dmr_rec_p->fdv_ms;
                }
            }

            if (succ_cnt > 0)
            {
                avg_fd_ms = total_fd_ms / succ_cnt;
                avg_fdv_ms = total_fdv_ms / succ_cnt;
            }

            /* correct the minimum frame dealy since
             * no good record can be used to set the minimum frame delay
             */
            if (min_fd_ms == 0xffffffff)
            {
                min_fd_ms = 0;
            }

            *avg_fd_ms_p = avg_fd_ms;
            *min_fd_ms_p = min_fd_ms;
            *max_fd_ms_p = max_fd_ms;
            *avg_fdv_ms_p = avg_fdv_ms;
            *succ_cnt_p  = succ_cnt;
            *total_cnt_p = dmm_ctrl_rec_p->counts;
            ret = TRUE;
        }
    }
    CFM_OM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextDmmReplyRec
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the next reply record of dmm.
 * INPUT    : recv_idx_p- pointer to content of record idx to get
 *                        (0 - to get first)
 * OUTPUT   : recv_idx_p- pointer to content of next record idx
 *            send_idx_p- pointer to content of send idx
 *            fd_ms_p   - pointer to content of frame delay in ms
 *            fdv_ms_p  - pointer to content of frame delay variation in ms
 *            res_p     - pointer to content of this record'result
 *                        CFM_TYPE_DMM_REPLY_REC_STATE_TIMEOUT  - dmr not received.
 *                        CFM_TYPE_DMM_REPLY_REC_STATE_RECEIVED - dmr     received
 *            is_succ_p - pointer to content of operation flag
 *                        TRUE  - this record is retrieved successfully
 *                        FALSE - try to get again later
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetNextDmmReplyRec(
                        UI32_T  *recv_idx_p,
                        UI32_T  *send_idx_p,
                        UI32_T  *fd_ms_p,
                        UI32_T  *fdv_ms_p,
                        UI8_T   *res_p,
                        BOOL_T  *is_succ_p)
{
    CFM_OM_DmmCtrlRec_T *dmm_ctrl_rec_p;
    UI32_T              tmp_idx, rec_idx;
    BOOL_T              ret = FALSE;

    if(NULL == recv_idx_p
        || NULL == fd_ms_p
        || NULL == res_p
        || NULL == is_succ_p
        )
        return FALSE;
    CFM_OM_LOCK();
    dmm_ctrl_rec_p = CFM_OM_GetDmmCtrlRecPtr();

    if (NULL != dmm_ctrl_rec_p)
    {
        tmp_idx = *recv_idx_p + 1;

        /* stop when all record is looped...
         */
        if (dmm_ctrl_rec_p->counts >= tmp_idx)
        {
            ret = TRUE;

            /* get the record out if reply is received...
             */
            if (dmm_ctrl_rec_p->cur_rcv_idx >= tmp_idx)
            {
                *is_succ_p = TRUE;
                *recv_idx_p     = tmp_idx;

                rec_idx = dmm_ctrl_rec_p->dmr_seq_ar[tmp_idx-1] -1;
                if ((0 <= rec_idx) && (rec_idx < CFM_TYPE_MAX_DMM_COUNT))
                {
                    *fd_ms_p   = dmm_ctrl_rec_p->dmr_rec_ar[rec_idx].frame_delay_ms;
                    *fdv_ms_p  = dmm_ctrl_rec_p->dmr_rec_ar[rec_idx].fdv_ms;
                    *res_p     = dmm_ctrl_rec_p->dmr_rec_ar[rec_idx].rec_state;
                    *send_idx_p= rec_idx +1;
                }
                else
                {
                    *res_p     = CFM_TYPE_DMM_REPLY_REC_STATE_TIMEOUT;
                }
            }
            else
            {
                *is_succ_p = FALSE;
            }
        }
    }
    CFM_OM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetThrpMeasureResult
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the throughput measure result.
 * INPUT    : None
 * OUTPUT   : real_send_p - the count of packets sent in one sec
 *            rcvd_1sec_p - the count of packets received in one sec
 *            rcvd_total_p- the count of packets received before timeout
 *            res_bmp_p   - the bitmap to indicate which data is retrieved
 *                          CFM_TYPE_THRPT_MEASURE_RES_SENDPKTS,
 *                          CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_1SEC,
 *                          CFM_TYPE_THRPT_MEASURE_RES_RECVPKTS_TOUT
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_GetThrpMeasureResult(
                    UI32_T  *real_send_p,
                    UI32_T  *rcvd_1sec_p,
                    UI32_T  *rcvd_total_p,
                    UI8_T   *res_bmp_p)
{
    CFM_OM_LbmCtrlRec_T     *lbm_ctrl_rec_p;
    BOOL_T                  ret = FALSE;

    if(NULL == real_send_p
        || NULL == rcvd_1sec_p
        || NULL == rcvd_total_p
        || NULL == res_bmp_p
        )
        return FALSE;
    CFM_OM_LOCK();
    lbm_ctrl_rec_p = CFM_OM_GetLbmCtrlRecPtr();

    if (NULL != lbm_ctrl_rec_p)
    {
        *real_send_p  = lbm_ctrl_rec_p->real_send_counts;
        *rcvd_1sec_p  = lbm_ctrl_rec_p->rcv_in_1sec;
        *rcvd_total_p = lbm_ctrl_rec_p->rcv_in_time + lbm_ctrl_rec_p->rcv_in_1sec;
        *res_bmp_p    = lbm_ctrl_rec_p->res_bmp;

        ret = TRUE;
    }
    CFM_OM_UNLOCK();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_AddError
 *-------------------------------------------------------------------------
 * PURPOSE  : This function add  the error to list
 * INPUT    : error_time - the error start time
 *            reason     - the error type
 *            mep_p      - the mep pointer
 *            rem_mac_p  - pointer to remote mac address to record
 * OUTPUT   : None
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_AddError(
                    UI32_T error_time,
                    CFM_TYPE_MEP_ConfigError_T reason,
                    CFM_OM_MEP_T *mep_p,
                    UI8_T *rem_mac_p)
{
    CFM_OM_ErrorListElement_T *error_p;
    I16_T ar_idx=-1;

    CFM_OM_LOCK();
    if(-1 == (ar_idx=CFM_OM_LocalGetFreeErrorAllocation()))
    {
        CFM_OM_UNLOCK();
        return FALSE;
    }

    error_p=&sharmem_data_p->error_list_ag[ar_idx];
    memset(error_p, 0, sizeof(CFM_OM_ErrorListElement_T));

    error_p->ar_idx=ar_idx;
    error_p->vid=mep_p->primary_vid;
    error_p->level=mep_p->md_p->level;
    error_p->mp_id=mep_p->identifier;

    if (NULL != rem_mac_p)
    {
        memcpy(error_p->mac_a, rem_mac_p, SYS_ADPT_MAC_ADDR_LEN);
    }
    error_p->md_ar_idx=mep_p->md_ar_idx;
    error_p->ma_ar_idx=mep_p->ma_ar_idx;
    error_p->lport=mep_p->lport;
    error_p->last_reason=reason;
    error_p->used=TRUE;

    switch(reason)
    {
        case CFM_TYPE_CONFIG_ERROR_LEAK:
            error_p->reason_bit_map|=(BIT_0);
            break;
        case CFM_TYPE_CONFIG_ERROR_VIDS:
            error_p->reason_bit_map|=(BIT_1);
            break;
        case CFM_TYPE_CONFIG_ERROR_EXCESSIVE_LEVELS:
            error_p->reason_bit_map|=(BIT_2);
            break;
        case CFM_TYPE_CONFIG_ERROR_OVERLAPPED_LEVELS:
            error_p->reason_bit_map|=(BIT_3);
            break;
        case CFM_TYPE_CONFIG_ERROR_AIS:
            error_p->reason_bit_map|=(BIT_4);
            break;
        default:
            break;
    }

    error_p->error_time =error_time;

    CFM_OM_LocalAddErrorsToErrorsList(error_p);
    CFM_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningCfmGlobalStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :This function get running cfm global status configuration
 * INPUT    : None
 * OUTPUT   : *cfmStatus_p - the CFM status pionter
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm enable
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningCfmGlobalStatus(
                                    CFM_TYPE_CfmStatus_T *cfmStatus_p )
{

    if(FALSE == CFM_OM_GetCFMGlobalStatus(cfmStatus_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(*cfmStatus_p == SYS_DFLT_CFM_GLOBAL_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

}/* End of CFM_OM_GetRunningCfmGlobalStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningCfmPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running cfm port  status configuration
 * INPUT    : lport              - the logical port
 * OUTPUT   : *cfm_port_Status_p - the cfm status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm port-enable
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningCfmPortStatus(
                              UI32_T lport,
                              CFM_TYPE_CfmStatus_T *cfm_port_Status_p )
{

    if(FALSE == CFM_OM_GetCFMPortStatus(lport, cfm_port_Status_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(*cfm_port_Status_p == SYS_DFLT_CFM_GLOBAL_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningCfmPortStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningCrossCheckStartDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running global cross check start delayconfiguration
 * INPUT    : None
 * OUTPUT   :*delay_p - the cross check start delay pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm mep crosscheck start-delay delay
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningCrossCheckStartDelay(
                                        UI32_T *delay_p)
{

    if(FALSE == CFM_OM_GetCrossCheckStartDelay(delay_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(*delay_p == SYS_DFLT_CFM_CROSSCHECK_START_DELAY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningCrossCheckStartDelay*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningLinktraceHoldTime
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running linktrace hold time configuration
 * INPUT    : None
 * OUTPUT   : *hold_time_p - the linnk trace reply entries hold time pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm linktrace cache hold-time minutes
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningLinktraceHoldTime(
                                        UI32_T *hold_time_p)
{

    if(FALSE == CFM_OM_GetLinkTraceCacheHoldTime(hold_time_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *hold_time_p = *hold_time_p/60;
    if(*hold_time_p == SYS_DFLT_CFM_LINKTRACE_HOLD_TIME)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

}/* End of CFM_OM_GetRunningLinktraceHoldTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningLinktraceCacheSize
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running linktrace cache size configuration
 * INPUT    : None
 * OUTPUT   : size_p - the link trace cache size pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm linktrace cache size entries
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningLinktraceCacheSize(
                                        UI32_T *size_p)
{

    if(FALSE == CFM_OM_GetLinkTraceCacheSize(size_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(*size_p == SYS_DFLT_CFM_LINKTRACE_CACHE_SIZE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningLinktraceCacheSize*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningLinktraceCacheStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running linktrace cache status configuration
 * INPUT    : None
 * OUTPUT   : status_p - the link trace cache status pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm linktrace cache
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningLinktraceCacheStatus(
                                CFM_TYPE_LinktraceStatus_T *status_p)
{

    if(FALSE == CFM_OM_GetLinkTraceCacheStatus(status_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(*status_p == SYS_DFLT_CFM_LINKTRACE_CACHE_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningLinktraceCacheStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningSnmpCcTrap
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running linktrace hold time configuration
 * INPUT    : None
 * OUTPUT   : *changed_trap_p - the byte bitmap store the present the traps type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : enable_traps
 *           |76543210|
 *            bit 7 : ALL, bit 6: MEP_UP, bit 5:MEP_DOWN,bit 4:CONFIG,bit 3:LOOP
 *  snmp-server enable traps ethernet cfm cc [mep-up|mep-down|config|loop]
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningSnmpCcTrap(
                        UI8_T *changed_trap_p)
{
    BOOL_T is_enabled=FALSE, trap_config_changed=FALSE;

    if (NULL == changed_trap_p)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *changed_trap_p = 0;
    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_ALL, &is_enabled);
    if(is_enabled != SYS_DFLT_CFM_SNMP_TRAP_CC_ALL)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=(1<<7);
    }

    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_MEP_UP, &is_enabled);
    if( is_enabled !=SYS_DFLT_CFM_SNMP_TRAP_CC_MEP_UP)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=1<<6;
    }

    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN, &is_enabled);
    if(is_enabled!= SYS_DFLT_CFM_SNMP_TRAP_CC_MEP_DOWN)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=1<<5;
    }

    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_CONFIG, &is_enabled);
    if(is_enabled!= SYS_DFLT_CFM_SNMP_TRAP_CC_CONFIG)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=1<<4;
    }

    CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_LOOP, &is_enabled);
    if( is_enabled!= SYS_DFLT_CFM_SNMP_TRAP_CC_LOOP)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=1<<3;
    }

    if(FALSE == trap_config_changed )
    {
         return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningSnmpCcTrap*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningSnmpCrossCheckTrap
 *-------------------------------------------------------------------------
 * PURPOSE  :This function get running linktrace hold time configuration
 * INPUT    : None
 * OUTPUT   : *enabledt_traps_p - the byte bitmap store the present the traps type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : enable_traps
 *           |76543210|
 *            bit 7 : ALL, bit 6: MA_UP, bit 5:MEP_MISSING,bit 4:MEP_UNKNOWN,bit
 *  snmp-server enable traps ethernet cfm crosscheck [mep-unknown | mep-missing | MA-up]
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningSnmpCrossCheckTrap(
                                 UI8_T *changed_trap_p)
{
    BOOL_T is_enabled=FALSE,trap_config_changed=FALSE;

    if (NULL == changed_trap_p)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *changed_trap_p = 0;
    CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL, &is_enabled);
    if(is_enabled!= SYS_DFLT_CFM_SNMP_TRAP_CROSS_CHECK_ALL)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=(1<<7);
    }

    CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP, &is_enabled);
    if(is_enabled!= SYS_DFLT_CFM_SNMP_TRAP_CROSS_CHECK_MA_UP)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=1<<6;
    }

    CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING, &is_enabled);
    if(is_enabled!= SYS_DFLT_CFM_SNMP_TRAP_CROSS_CHECK_MEP_MISSING)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=1<<5;
    }

    CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN, &is_enabled);
    if(is_enabled!= SYS_DFLT_CFM_SNMP_TRAP_CROSS_CHECK_UNKNOWN)
    {
        trap_config_changed=TRUE;
        *changed_trap_p|=1<<4;
    }

    if(FALSE == trap_config_changed )
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningSnmpCrossCheckTrap*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningFaultNotifyLowestPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running fault notify lowest priority configuration
 * INPUT    : None
 * OUTPUT   : priority_p - the fault notify lowest priority pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : mep fault-notify  lowest-priority priority
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningFaultNotifyLowestPriority(
                                UI32_T md_index,
                                CFM_TYPE_FNG_LowestAlarmPri_T *priority_p)
{

    if(FALSE == CFM_OM_GetFaultNotifyLowestPriority(md_index, priority_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(*priority_p == SYS_DFLT_CFM_FNG_LOWEST_ALARM_PRI)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningFaultNotifyLowestPriority*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningCCmStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running ccm status configuration
 * INPUT    : md_index - the md index
 * OUTPUT   : *ma_index - the ma index
 *          ma_name_p - the ma name
 *          *status_p - the CCM transmit status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm cc enable ma ma-name
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningCCmStatus(
                                  UI32_T md_index,
                                  UI32_T *ma_index,
                                  UI8_T ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
                                  CFM_TYPE_CcmStatus_T *status_p)
{
    CFM_OM_MA_T *ma_p=NULL;

    if(NULL == (ma_p=CFM_OM_GetNextMaByMaIndex(md_index, *ma_index)))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(ma_p->ccm_status == SYS_DFLT_CFM_CCM_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    *status_p = ma_p->ccm_status;
    *ma_index = ma_p->index;
    memcpy(ma_name_a, ma_p->name_a, ma_p->name_length);

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningCCmStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetnNextRunningCCmInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running ccm interval  configuration
 * INPUT    : ma_index - the md index
 * OUTPUT   : *ma_index - the ma index
 *          ma_name_p-the ma name
 *          *interval_p - the cc interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : ethernet cfm cc ma ma-name interval interval-level
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningCCmInterval(
                                    UI32_T md_index,
                                    UI32_T *ma_index_p,
                                    UI8_T ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH],
                                    CFM_TYPE_CcmInterval_T *interval_p)
{
    CFM_OM_MA_T *ma_p=NULL;

    if(NULL == (ma_p=CFM_OM_GetNextMaByMaIndex(md_index, *ma_index_p)))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(ma_p->ccm_status == SYS_DFLT_CFM_CCM_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    *interval_p = ma_p->ccm_interval;
    *ma_index_p = ma_p->index;
    memcpy(ma_name_a, ma_p->name_a, ma_p->name_length);

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of CFM_OM_GetRunningCCmInterval*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRunningMd
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running md configuration
 * INPUT    : *md_index_p - the md index pointer
 * OUTPUT   : *md_info_p - the md next info  pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :  ethernet cfm domain index index name domain-name level level-id
 *             mep archive-hold-time minutes
 *             mep fault-notify  lowest-priority priority
 *             mep fault-notify  alarm-time time
 *             mep fault-notify  reset-time timere
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningMd(
                           UI32_T *md_index_p,
                           CFM_OM_MdInfo_T *md_info_p)
{
    CFM_OM_MD_T *md_p=NULL;

    if(NULL == md_index_p
        || NULL == md_info_p)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(NULL == (md_p=CFM_OM_GetNextMdByIndex(*md_index_p)))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CFM_OM_LocalFillMdInfo(md_p, md_info_p);
    *md_index_p = md_p->index;
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRunningMa
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running md configuration
 * INPUT    : md_index    - the md index
 *            *ma_index   - the currnet ma index
 * OUTPUT   : * ma_index     - the next ma index
 *          *ma_info_p       - the next ma info
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :ma index index name name vlan vid-list
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningMa(
                        UI32_T md_index,
                        UI32_T *ma_index,
                        CFM_OM_MaInfo_T *ma_info_p)
{
    CFM_OM_MA_T *ma_p=NULL;

    if(NULL == ma_index
        || NULL == ma_info_p)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(NULL == (ma_p=CFM_OM_GetNextMaByMaIndex(md_index, *ma_index)))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CFM_OM_LocalFillMaInfo(ma_p, ma_info_p);
    *ma_index = ma_p->index;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /*End of CFM_OM_GetNextRunningMa*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetNextRunnningMep
 *-------------------------------------------------------------------------
 * PURPOSE  :This function get running md configuration
 * INPUT    : *md_index - the md index pointer
 *            *ma_index - the ma index pointer
 *            lport     - the logical port*
 * OUTPUT   : *md_index - the next md index pointer
 *            *ma_index - the next ma index pointer
 *            *ret_mep_p    - the mep info
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :ethernet cfm mep mpid mpid ma ma-name [up]
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunnningMep(
                             UI32_T *md_index_p,
                             UI32_T *ma_index_p,
                             UI32_T lport,
                             CFM_OM_MepInfo_T *ret_mep_p)
{
    CFM_OM_MEP_T *mp_p=&mep_tmp3_g;

    if(NULL == md_index_p
        || NULL == ma_index_p
        || NULL == ret_mep_p)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(FALSE == CFM_OM_GetNextMep(*md_index_p, *ma_index_p, 0, lport, CFM_OM_LPORT_MD_MA_KEY, mp_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(mp_p->lport != lport)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CFM_OM_LocalFillMepInfo(mp_p, ret_mep_p);

    *md_index_p=mp_p->md_index;
    *ma_index_p=mp_p->ma_index;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of CFM_OM_GetNextRunnningMep*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_MGR_GetRunningNextCrossCheckMpId
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get running md configuration
 * INPUT    : md_index     - the md index
 *            *ma_index_p   - the ma index pointer
 *            *remtoe_mep_p - the remote mep identifier pointer
 *            *ma_name_a    - the ma name array
 *            current_time  - current time
 * OUTPUT   : *ma_index_p   - the ma next index pointer
 *            *remtoe_mep_p - the remote mep next identifier pointer
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :  mep crosscheck mpid id ma ma-name
 *-------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetNextRunningCrossCheckMpIdInfo(
    UI32_T  md_index,
    UI32_T  *ma_index_p,
    UI32_T  *remote_mep_id,
    UI32_T  current_time,
    UI8_T   ma_name_a[CFM_TYPE_MA_MAX_NAME_LENGTH+1])
{
    UI32_T                          md_tmp_idx=md_index;
    CFM_OM_RemoteMepCrossCheck_T    remote_mep;

    if(FALSE == CFM_OM_GetNextRemoteMepInfo(&md_tmp_idx, ma_index_p, remote_mep_id, current_time, &remote_mep))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    strncpy((char *) ma_name_a, (char *)remote_mep.ma_name_a, CFM_TYPE_MA_MAX_NAME_LENGTH);

    if(md_index != md_tmp_idx)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of CFM_OM_GetNextRunningCrossCheckMpIdInfo*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningAisPeriod
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the ma ais period running configuration
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :*period_p - the ais period pointer
 * RETUEN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS            - have change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL           - fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningAisPeriod(
                                UI32_T md_index,
                                UI32_T ma_index,
                                UI32_T *period_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    if(FALSE == CFM_OM_GetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(ma_p->ais_period == SYS_DFLT_CFM_AIS_PERIOD)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    *period_p = ma_p->ais_period;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of CFM_OM_GetRunningAisPeriod*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningAisLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the running config ais level
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :*ais_level_p - the ais level pointer
 * RETUEN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS            - have change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL           - fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningAisLevel(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_MdLevel_T *ais_level_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    if(FALSE == CFM_OM_GetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(ma_p->ais_send_level == SYS_DFLT_CFM_AIS_LEVEL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    *ais_level_p = ma_p->ais_send_level;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of CFM_OM_GetRunningAisLevel*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningAisSuppressStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the running configure
 * INPUT    : md_index      - the md index
 *                ma_index      - the ma index
 * OUTPUT   :*ais_supress_status_p - the ais suppress status pointer
 * RETUEN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS            - have change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL           - fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningAisSuppressStatus(
                                UI32_T md_index,
                                UI32_T ma_index,
                                CFM_TYPE_AIS_STATUS_T *ais_supress_status_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    if(FALSE == CFM_OM_GetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(ma_p->ais_supress_status== SYS_DFLT_CFM_AIS_SUPRESS_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    *ais_supress_status_p= ma_p->ais_supress_status;
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of CFM_OM_GetRunningAisSuppressStatus*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetRunningAisStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :This function get the ais running status
 * INPUT    : md_index     - the md index
 *            ma_index     - the ma index
 * OUTPUT   :*ais_status_p - the ais status pointer
 * RETUEN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS            - have change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL           - fail
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI32_T CFM_OM_GetRunningAisStatus(
                            UI32_T md_index,
                            UI32_T ma_index,
                            CFM_TYPE_AIS_STATUS_T *ais_status_p)
{
    CFM_OM_MD_T *md_p=NULL;
    CFM_OM_MA_T *ma_p = NULL;

    if(FALSE == CFM_OM_GetMdMaByMdMaIndex(md_index, ma_index, &md_p, &ma_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if(ma_p->ais_send_level == SYS_DFLT_CFM_AIS_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    *ais_status_p= ma_p->ais_status;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of CFM_OM_GetRunningAisStatus*/


/* for delay measurement
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetDmmCtrlRecPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : To get pointer to global dmm control record.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : pointer to global dmm control record
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_DmmCtrlRec_T *CFM_OM_GetDmmCtrlRecPtr(void)
{
    return &sharmem_data_p->dmm_ctr_rec_g;
}
/* end for delay measurement
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetLbmCtrlRecPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : To get pointer to global lbm control record.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : pointer to global lbm control record
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
CFM_OM_LbmCtrlRec_T *CFM_OM_GetLbmCtrlRecPtr(void)
{
    return &sharmem_data_p->lbm_ctr_rec_g;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_CnvOidToData
 *-------------------------------------------------------------------------
 * PURPOSE  : To convert oid array to asn.1 encoded array.
 * INPUT    : in_oid_p        - pointer to input oid array
 *            in_oidlen       - number of input oid array, in bytes
 *            inout_data_p    - maximum number of output asn.1 encoded array, in bytes
 * OUTPUT   : out_data_p      - pointer to output asn.1 encoded array
 *            inout_datalen_p - number of output asn.1 encoded array used, in bytes
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_OM_CnvOidToData(
    UI32_T  *in_oid_p,
    UI32_T  in_oidlen,
    UI8_T   *out_data_p,
    UI8_T   *inout_datalen_p)
{
    UI32_T  cur_in_idx = 0, cur_val = 0, cur_out_idx= 2;
    BOOL_T  is_ok = FALSE;

    if ((NULL != in_oid_p) && (NULL != out_data_p) && (NULL != inout_datalen_p))
    {
        while (cur_in_idx < in_oidlen)
        {
            if (cur_out_idx >= *inout_datalen_p)
            {
                break;
            }
            if( cur_in_idx == 0 )
            {
                cur_val = 40 * (in_oid_p[cur_in_idx]);
            }
            else if( cur_in_idx == 1 )
            {
                cur_val += (in_oid_p[cur_in_idx]);
                out_data_p[cur_out_idx++] = cur_val;
            }
            else
            {
                I32_T   shift_cnt =0;

                cur_val = in_oid_p[cur_in_idx];
                while (cur_val > 127)
                {
                    cur_val >>= 7;
                    shift_cnt ++;
                }

                cur_val = in_oid_p[cur_in_idx];
                while (shift_cnt >= 0)
                {
                    out_data_p[cur_out_idx] = (cur_val >> (7 * shift_cnt)) & 0x7f;
                    if (shift_cnt > 0)
                    {
                        out_data_p[cur_out_idx] |= 0x80;
                    }
                    cur_out_idx++;

                    if (cur_out_idx >= *inout_datalen_p)
                    {
                        /* to break the outer while (cur_in_idx < in_oidlen)
                         *  and return FALSE
                         */
                        cur_in_idx = in_oidlen +1;
                        break;
                    }

                    shift_cnt--;
                }
            }
            cur_in_idx++;
        }

        if(cur_in_idx == in_oidlen)
        {
            is_ok = TRUE;
            *inout_datalen_p = cur_out_idx;
            out_data_p[0] = 0x06;             /* type: OID */
            out_data_p[1] = cur_out_idx -2;   /* len       */
        }
    }

   // CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_OM,
    //    "ret-%d, in-%ld, out-%ld", is_ok, cur_in_idx, cur_out_idx);

    return is_ok;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_LocalGetCurrentMaxErrPri
 *-------------------------------------------------------------------------
 * PURPOSE  : To get current highest defect priority of a MEP.
 * INPUT    : mep_p         - pointer to MEP to get
 *            is_skip_rdi   - TRUE to ignore the RDI defect
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_FNG_HighestDefectPri_T
 * NOTE     : will not check mep->lowest_priority_defect
 *-------------------------------------------------------------------------
 */
static CFM_TYPE_FNG_HighestDefectPri_T
CFM_OM_LocalGetCurrentMaxErrPri(
    CFM_OM_MEP_T    *mep_p,
    BOOL_T          is_skip_rdi)
{
    if (TRUE == mep_p->xcon_ccm_defect)
        return CFM_TYPE_FNG_HIGHEST_DEFECT_XCON_CCM;

    if (TRUE == mep_p->error_ccm_defect)
        return CFM_TYPE_FNG_HIGHEST_DEFECT_ERROR_CCM;

    if (TRUE == mep_p->some_rmep_ccm_defect)
        return CFM_TYPE_FNG_HIGHEST_DEFECT_REMOTE_CCM;

    if (TRUE == mep_p->err_mac_status)
        return CFM_TYPE_FNG_HIGHEST_DEFECT_MAC_STATUS;

    if (  (FALSE == is_skip_rdi)
        &&(TRUE == mep_p->some_rdi_defect)
       )
        return CFM_TYPE_FNG_HIGHEST_DEFECT_RDI_CCM;

    return CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_GetCurrentMaxErrPri
 *-------------------------------------------------------------------------
 * PURPOSE  : To get current highest defect priority of a MEP.
 * INPUT    : mep_p         - pointer to MEP to get
 *            is_skip_rdi   - TRUE to ignore the RDI defect
 * OUTPUT   : None
 * RETURN   : CFM_TYPE_FNG_HighestDefectPri_T
 * NOTE     : will check mep->lowest_priority_defect
 *-------------------------------------------------------------------------
 */
CFM_TYPE_FNG_HighestDefectPri_T
CFM_OM_GetCurrentMaxErrPri(
    CFM_OM_MEP_T    *mep_p,
    BOOL_T          is_skip_rdi)
{
    CFM_TYPE_FNG_HighestDefectPri_T     cur_max_err_pri;

    cur_max_err_pri = CFM_OM_LocalGetCurrentMaxErrPri(mep_p, is_skip_rdi);
    /* according to 802.1ag-2007.pdf, 20.9.3 MAdefectIndication
     * MAdefectIndication is true if and only if, for one or more of the
     * variables someRDIdefect, someRMEPCCMdefect, someMACstatusDefect,
     * errorCCMdefect, or xconCCMdefect, that variable is true and the
     * corresponding priority of that variable greater than or equal to
     * the value of the variable lowestAlarmPri.
     */
    if ((long)cur_max_err_pri < (long)mep_p->lowest_priority_defect)
    {
        cur_max_err_pri = CFM_TYPE_FNG_HIGHEST_DEFECT_PRI_NONE;
    }
    return cur_max_err_pri;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_CnvDataToOid
 *-------------------------------------------------------------------------
 * PURPOSE  : To convert asn.1 encoded array to oid array.
 * INPUT    : in_data_p      - pointer to input asn.1 encoded array
 *            in_datalength  - number of input asn.1 encoded array, in bytes
 *            inout_oidlen_p - maximum number of output oid array, in bytes
 * OUTPUT   : out_oid_p      - pointer to output oid array
 *            inout_oidlen_p - number of output oid array used, in bytes
 * RETURN   : TRUE   - success
 *            FALSE  - fail
 * NOTE     : 1. if output oid array is not enough, the output will be
 *               truncated and return FALSE.
 *-------------------------------------------------------------------------
 */
static BOOL_T CFM_OM_CnvDataToOid(
    UI8_T   *in_data_p,
    UI32_T  in_datalen,
    UI32_T  *out_oid_p,
    UI32_T  *inout_oidlen_p)
{
    UI32_T  cur_in_idx = 0, cur_val = 0, cur_out_idx= 0, out_max_len;
    BOOL_T  is_ok = FALSE;

    if ((NULL != in_data_p) && (NULL != out_oid_p) && (NULL != inout_oidlen_p))
    {
        out_max_len = *inout_oidlen_p / sizeof(UI32_T);

        while (cur_in_idx < in_datalen)
        {
            if (cur_out_idx >= out_max_len)
            {
                is_ok = FALSE;
                break;
            }

            if (cur_in_idx == 0)
            {
                /* to remove compiler warning */
                #if 0
                unsigned char cl = ((in_data_p[cur_in_idx] & 0xC0) >> 6) & 0x03;
                #endif
            }
            else if (cur_in_idx == 1)
            {
                if (in_datalen - 2 != in_data_p[cur_in_idx])
                {
                    break;
                }
            }
            else if (cur_in_idx == 2)
            {
                if (cur_out_idx+2 >= out_max_len)
                {
                    is_ok = FALSE;
                    break;
                }

                out_oid_p[cur_out_idx++] = in_data_p[cur_in_idx] /40;
                out_oid_p[cur_out_idx++] = in_data_p[cur_in_idx] %40;
                is_ok   = TRUE;
                cur_val = 0;
            }
            else if ((in_data_p[cur_in_idx] & 0x80) != 0)
            {
                cur_val *= 128;
                cur_val += (in_data_p[cur_in_idx] & 0x7F);
                is_ok = FALSE;
            }
            else
            {
                cur_val *= 128;
                cur_val += in_data_p[cur_in_idx];
                out_oid_p[cur_out_idx++] = cur_val;
                is_ok   = TRUE;
                cur_val = 0;
            }
            cur_in_idx++;
        }
    }

    /* return the converted data even fail
     */
    if (NULL != inout_oidlen_p)
        *inout_oidlen_p = cur_out_idx * sizeof (UI32_T);

  //  CFM_BD_MSG(CFM_BACKDOOR_DEBUG_FLAG_OM,
    //    "ret-%d, in-%ld, out-%ld", is_ok, cur_in_idx, cur_out_idx);

    return is_ok;
}

#endif /*#if (SYS_CPNT_CFM == TRUE)*/

