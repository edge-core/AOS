/* MODULE NAME: pfc_om.c
 * PURPOSE:
 *   Definitions of OM APIs for PFC
 *   (IEEE Std 802.1Qbb, Priority-based Flow Control).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   10/15/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_type.h"
#include "sys_dflt.h"

#if (SYS_CPNT_PFC == TRUE)

#include "sysfun.h"
#include "pfc_backdoor.h"
#include "pfc_om.h"
#include "l_stdlib.h"
#include "l_rstatus.h"

#if (PFC_TYPE_BUILD_LINUX == TRUE)
#include "sys_bld.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define PFC_OM_SYS_STRUCT          0
#define PFC_OM_ADM_STRUCT          1
#define PFC_OM_OPR_STRUCT          2
#define PFC_OM_NAN_STRUCT          3

#define PFC_OM_RW_FLAG_RO        0x1
#define PFC_OM_RW_FLAG_WO        0x2
#define PFC_OM_RW_FLAG_RW        0x3
#define PFC_OM_RW_FLAG_NA        0x0

/* MACRO FUNCTIONS DECLARACTION
 */
#define PFC_OM_SETFLDINFO(rtype,rw,s,memb)             \
                {   .rec_type = rtype,                 \
                    .rw_flag  = rw,                    \
                    .beg = PFC_TYPE_OFFSETOF(s,memb),  \
                    .len = PFC_TYPE_SIZEOF(s,memb) }

#define PFC_OM_SETFLDINFO_NA(rtype, rw)                \
                {   .rec_type = rtype,                 \
                    .rw_flag  = rw,                    \
                    .beg = 0,                          \
                    .len = 0 }

#if (PFC_TYPE_BUILD_LINUX == TRUE)
    #define PFC_OM_LOCK()              orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(pfc_om_sem_id)
    #define PFC_OM_UNLOCK()            SYSFUN_OM_LEAVE_CRITICAL_SECTION(pfc_om_sem_id, orig_priority)
#else
    #define PFC_OM_LOCK()              SYSFUN_OM_ENTER_CRITICAL_SECTION()
    #define PFC_OM_UNLOCK()            SYSFUN_OM_LEAVE_CRITICAL_SECTION()
#endif

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T   rec_type;
    UI8_T   rw_flag;
    UI8_T   beg;
    UI8_T   len;
}   PFC_OM_FldInfo_T;

typedef struct PFC_OM_ShmemData_S
{
    UI32_T                  pfc_debug_flag;
    PFC_TYPE_PortCtrlRec_T  pctrl_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    BOOL_T                  pfc_is_cos_ok;
//    PFC_OM_FldInfo_T               pctrl_fldinfo[PFC_TYPE_FLDE_MAX];
 }   PFC_OM_ShmemData_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T PFC_OM_LocalIsAnyPortOperEnabled(void);


/* STATIC VARIABLE DECLARATIONS
 */
#if (PFC_TYPE_BUILD_LINUX == TRUE)
static UI32_T                   pfc_om_sem_id;
static UI32_T                   orig_priority;
static PFC_OM_ShmemData_T      *pfc_sharmem_data_p;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
#if (PFC_TYPE_BUILD_LINUX == TRUE)
/*---------------------------------------------------------------------------------
 * FUNCTION - PFC_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for PFC OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void PFC_OM_AttachSystemResources(void)
{
    pfc_sharmem_data_p = (PFC_OM_ShmemData_T*) SYSRSC_MGR_GetShMem(SYSRSC_MGR_PFC_OM_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_PFC_OM, &pfc_om_sem_id);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  -  shared memory segment id
 *          seglen_p -  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:   This function is called in SYSRSC_MGR_CreateSharedMemory().
 *-------------------------------------------------------------------------
 */
void PFC_OM_GetShMemInfo(
    SYSRSC_MGR_SEGID_T  *segid_p,
    UI32_T              *seglen_p)
{
    *segid_p  = SYSRSC_MGR_PFC_OM_SHMEM_SEGID;
    *seglen_p = sizeof(*pfc_sharmem_data_p);
}

#endif /* #if (PFC_TYPE_BUILD_LINUX == TRUE) */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_ClearOM
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_OM_ClearOM(void)
{
    memset(pfc_sharmem_data_p->pctrl_ar, 0, sizeof(pfc_sharmem_data_p->pctrl_ar));

    /* default cos should be ok */
    pfc_sharmem_data_p->pfc_is_cos_ok = TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_InitOM
 *--------------------------------------------------------------------------
 * PURPOSE: To init the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_OM_InitOM(void)
{
#if 0
    static PFC_OM_FldInfo_T    tmp_pctrl_fldinfo[PFC_TYPE_FLDE_MAX] =
        {
            [PFC_TYPE_FLDE_GLOBAL_MSG_IVL]     = PFC_OM_SETFLDINFO(PFC_OM_SYS_STRUCT, PFC_OM_RW_FLAG_RW, PFC_TYPE_SysCtrlRec_T, msg_invl),
            [PFC_TYPE_FLDE_GLOBAL_EN_PCNT]     = PFC_OM_SETFLDINFO(PFC_OM_SYS_STRUCT, PFC_OM_RW_FLAG_RW, PFC_TYPE_SysCtrlRec_T, en_port_cnt),
            [PFC_TYPE_FLDE_PORT_ENABLE]        = PFC_OM_SETFLDINFO(PFC_OM_ADM_STRUCT, PFC_OM_RW_FLAG_RW, PFC_TYPE_PortAdmRec_T, is_enable),
            [PFC_TYPE_FLDE_PORT_AGGRESSIVE]    = PFC_OM_SETFLDINFO(PFC_OM_ADM_STRUCT, PFC_OM_RW_FLAG_RW, PFC_TYPE_PortAdmRec_T, is_aggressive),
            [PFC_TYPE_FLDE_PORT_OPR_STATE]     = PFC_OM_SETFLDINFO(PFC_OM_OPR_STRUCT, PFC_OM_RW_FLAG_RW, PFC_TYPE_PortOprRec_T, opr_state),
            [PFC_TYPE_FLDE_PORT_BID_STATE]     = PFC_OM_SETFLDINFO(PFC_OM_OPR_STRUCT, PFC_OM_RW_FLAG_RW, PFC_TYPE_PortOprRec_T, bid_state),
            [PFC_TYPE_FLDE_PORT_NERB_DEV_CNT]  = PFC_OM_SETFLDINFO(PFC_OM_OPR_STRUCT, PFC_OM_RW_FLAG_RO, PFC_TYPE_PortOprRec_T, nebr_dev_num),
            [PFC_TYPE_FLDE_PORT_SEQ_NUM]       = PFC_OM_SETFLDINFO(PFC_OM_OPR_STRUCT, PFC_OM_RW_FLAG_RW, PFC_TYPE_PortOprRec_T, seq_nbr),
            [PFC_TYPE_FLDE_PORT_MSG_INVL]      = PFC_OM_SETFLDINFO_NA(PFC_OM_NAN_STRUCT, PFC_OM_RW_FLAG_NA),
            [PFC_TYPE_FLDE_PORT_TIMEOUT]       = PFC_OM_SETFLDINFO_NA(PFC_OM_NAN_STRUCT, PFC_OM_RW_FLAG_NA),
            [PFC_TYPE_FLDE_PORT_ADM_DISABLE]   = PFC_OM_SETFLDINFO_NA(PFC_OM_NAN_STRUCT, PFC_OM_RW_FLAG_NA),
            [PFC_TYPE_FLDE_PORT_NXT_PORT]      = PFC_OM_SETFLDINFO_NA(PFC_OM_NAN_STRUCT, PFC_OM_RW_FLAG_NA),
            [PFC_TYPE_FLDE_PORT_NXT_NEBR_DEV]  = PFC_OM_SETFLDINFO_NA(PFC_OM_NAN_STRUCT, PFC_OM_RW_FLAG_NA),
            [PFC_TYPE_FLDE_PORT_NEBR_BY_ID]    = PFC_OM_SETFLDINFO_NA(PFC_OM_NAN_STRUCT, PFC_OM_RW_FLAG_NA),
            [PFC_TYPE_FLDE_PORT_NXT_NEBR_BY_ID]= PFC_OM_SETFLDINFO_NA(PFC_OM_NAN_STRUCT, PFC_OM_RW_FLAG_NA),
        };
#endif /* #if 0 */

    PFC_OM_ClearOM();

    pfc_sharmem_data_p->pfc_debug_flag =0;
//    memcpy(pfc_sharmem_data_p->pctrl_fldinfo, tmp_pctrl_fldinfo, sizeof(tmp_pctrl_fldinfo));
}

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE:   Initiate the semaphore for PFC objects.
 * INPUT  :   None
 * OUTPUT :   None
 * RETURN :   None
 * NOTE   :   This function is invoked in PFC_MGR_InitiateSystemResources.
 *-------------------------------------------------------------------------
 */
void PFC_OM_InitSemaphore(void)
{
}
#endif

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_CopyPortConfigTo
 *--------------------------------------------------------------------------
 * PURPOSE: To copy one config entry from specified src_lport to dst_lport.
 * INPUT  : src_lport - 1-based source      lport to copy config from
 *          dst_lport - 1-based destination lport to copy config to
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PFC_OM_CopyPortConfigTo(
    UI32_T  src_lport,
    UI32_T  dst_lport)
{
    BOOL_T  ret = FALSE;

    if (  ((1 <= src_lport) && (src_lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
        &&((1 <= dst_lport) && (dst_lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
       )
    {
        PFC_OM_LOCK();
        memcpy (&pfc_sharmem_data_p->pctrl_ar[dst_lport-1],
                &pfc_sharmem_data_p->pctrl_ar[src_lport-1],
                sizeof(pfc_sharmem_data_p->pctrl_ar[0]));
        PFC_OM_UNLOCK();

        ret = TRUE;
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_GetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To get the data by specified field id and lport.
 * INPUT  : lport    - lport to set
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 * OUTPUT : data_p   - pointer to output data
 * RETURN : TRUE/FALSE
 * NOTE   : 1. all output data are represented in UI32_T
 *          2. caller need to guarantee the data length is enough
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_OM_GetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    BOOL_T  ret = FALSE;

    if (NULL != data_p)
    {
        if (0 == lport)
        {
            PFC_OM_LOCK();
            switch (field_id)
            {
            case PFC_TYPE_FLDE_GLOBAL_IS_COS_OK:
                *((UI32_T *) data_p) = pfc_sharmem_data_p->pfc_is_cos_ok;
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_GLOBAL_MAX_TC_CAP:
                /* for current design,
                 * PFC can be enabled for 8 priorities/Tcs
                 */
                *((UI32_T *) data_p) = 8;
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_GLOBAL_IS_ANY_PORT_OP_EN:
                *((UI32_T *) data_p) = PFC_OM_LocalIsAnyPortOperEnabled();
                ret = TRUE;
                break;
            }
            PFC_OM_UNLOCK();
        }
        else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            PFC_TYPE_PortCtrlRec_T  *pfc_pctrl_p;

            PFC_OM_LOCK();

            pfc_pctrl_p = &pfc_sharmem_data_p->pctrl_ar[lport-1];

            switch (field_id)
            {
            case PFC_TYPE_FLDE_PORT_ENABLE:
                if (  (PFC_TYPE_PMODE_ON   == pfc_pctrl_p->mode_adm)
                    ||(PFC_TYPE_PMODE_AUTO == pfc_pctrl_p->mode_adm)
                   )
                {
                    *((UI32_T *) data_p) = TRUE;
                }
                else
                {
                    *((UI32_T *) data_p) = FALSE;
                }
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_PORT_MODE_ADM:
                *((UI32_T *) data_p) = pfc_pctrl_p->mode_adm;
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_PORT_MODE_OPR:
                *((UI32_T *) data_p) = pfc_pctrl_p->mode_opr;
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_PORT_PRI_EN_ADM:
            case PFC_TYPE_FLDE_PORT_PRI_EN_ADM_ADD:
            case PFC_TYPE_FLDE_PORT_PRI_EN_ADM_REM:
                *((UI32_T *) data_p) = pfc_pctrl_p->en_pri_bmp_adm;
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_PORT_PRI_EN_OPR:
                *((UI32_T *) data_p) = pfc_pctrl_p->en_pri_bmp_opr;
                ret = TRUE;
                break;
    #if 0
            case PFC_TYPE_FLDE_PORT_CLR_STATIS:
            case PFC_TYPE_FLDE_PORT_STATIS:
            case PFC_TYPE_FLDE_PORT_LDA:
    #endif
            }

            PFC_OM_UNLOCK();
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_SetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport.
 * INPUT  : lport - lport to set
 *                       0 - global data to set
 *                      >0 - lport  data to set
 *          field_id  - field id to set (PFC_TYPE_FieldId_E)
 *          data_p    - pointer to input data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : 1. all input data are represented in UI32_T
 *          2. caller need to guarantee the data length is enough
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_OM_SetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    BOOL_T  ret = FALSE;

    if (NULL != data_p)
    {
        if (0 == lport)
        {
            PFC_OM_LOCK();
            switch (field_id)
            {
            case PFC_TYPE_FLDE_GLOBAL_IS_COS_OK:
                pfc_sharmem_data_p->pfc_is_cos_ok = *((UI32_T *) data_p);
                ret = TRUE;
                break;
            }
            PFC_OM_UNLOCK();
        }
        else if (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            PFC_TYPE_PortCtrlRec_T  *pfc_pinfo_p;
            UI32_T                  tmp_data = *(UI32_T *) data_p;

            PFC_OM_LOCK();

            pfc_pinfo_p = &pfc_sharmem_data_p->pctrl_ar[lport-1];

            switch (field_id)
            {
            case PFC_TYPE_FLDE_PORT_MODE_ADM:
    #if 0
                SWCTRL_OM_PFC_UpdatePortEnableCounter(
                        tmp_data, pfc_pinfo_p->mode_adm);
    #endif /* #if 0 */

                pfc_pinfo_p->mode_adm = tmp_data;
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_PORT_MODE_OPR:
                pfc_pinfo_p->mode_opr = tmp_data;
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_PORT_PRI_EN_ADM:
                pfc_pinfo_p->en_pri_bmp_adm = tmp_data;
                ret = TRUE;
                break;
            case PFC_TYPE_FLDE_PORT_PRI_EN_OPR:
                pfc_pinfo_p->en_pri_bmp_opr = tmp_data;
                ret = TRUE;
                break;
    #if 0
            case PFC_TYPE_FLDE_PORT_CLR_STATIS:
            case PFC_TYPE_FLDE_PORT_STATIS:
            case PFC_TYPE_FLDE_PORT_LDA:
    #endif
            }

            PFC_OM_UNLOCK();
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_GetPortCtrlPtrByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To get pointer of port control record with specified lport.
 * INPUT  : lport - input lport (1-based)
 * OUTPUT : None
 * RETURN : NULL - failed
 *          pointer to content of port contrl record
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
PFC_TYPE_PortCtrlRec_T *PFC_OM_GetPortCtrlPtrByLport(
    UI32_T  lport)
{
    PFC_TYPE_PortCtrlRec_T *ret_p = NULL;

    if ((0 < lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        ret_p = &pfc_sharmem_data_p->pctrl_ar[lport -1];
    }

    return ret_p;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_IsDbgFlagOn
 *-------------------------------------------------------------------------
 * PURPOSE: To test if specified debug flag is set or not.
 * INPUT  : flag_enum - debug flag enum id to test
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_OM_IsDbgFlagOn(
    UI32_T  flag_enum)
{
    return (0 != PFC_TYPE_TST_BIT(
                    pfc_sharmem_data_p->pfc_debug_flag,
                    flag_enum));
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_GetDbgFlag
 *-------------------------------------------------------------------------
 * PURPOSE: To get the debug flag.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : debug flag
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
UI32_T PFC_OM_GetDbgFlag(void)
{
    return pfc_sharmem_data_p->pfc_debug_flag;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_SetDbgFlag
 *-------------------------------------------------------------------------
 * PURPOSE: To set the debug flag.
 * INPUT  : debug_flag - debug flag to set
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
void PFC_OM_SetDbgFlag(
    UI32_T  debug_flag)
{
    pfc_sharmem_data_p->pfc_debug_flag = debug_flag;
}


/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_OM_LocalIsAnyPortOperEnabled
 *-------------------------------------------------------------------------
 * PURPOSE: To check if any port is operational enabled.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T PFC_OM_LocalIsAnyPortOperEnabled(void)
{
    UI32_T  lport;
    BOOL_T  ret = FALSE;

    for (lport =0; lport <SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (PFC_TYPE_PMODE_ON == pfc_sharmem_data_p->pctrl_ar[lport].mode_opr)
        {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

#endif /* #if (SYS_CPNT_PFC == TRUE) */

