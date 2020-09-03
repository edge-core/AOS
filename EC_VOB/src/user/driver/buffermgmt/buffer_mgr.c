/* Module Name: BUFFER_MGR.h
 * Purpose:
 *  This module is responsible for the control of landing buffer
 *
 * Notes:
 *
 * History:
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2004.08.10  -- Erica Li,        Creation
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <string.h>
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_bld.h"
#include "l_cvrt.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "buffermgmt_init.h"
#include "buffer_mgr.h"

/* LOCAL DATATYPE DECLARATION
 */
#define BUFFER_MGR_EnterCriticalSection()   SYSFUN_TakeSem(buffer_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define BUFFER_MGR_LeaveCriticalSection()   SYSFUN_GiveSem(buffer_mgr_sem_id)

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    BOOL_T   is_allocated;
    UI8_T    buffer[BUFFERMGMT_INIT_BUFFER_SIZE];
} BUFFER_MGR_Shmem_Data_T;

/* LOCAL NAMEING CONSTANT DECLARATION
 */
static BUFFER_MGR_Shmem_Data_T *shmem_data_p;

static UI32_T   buffer_mgr_sem_id;

/* EXPORTED SUBPROGRAM BODIES
 */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_InitiateSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Initiate system resource for BUFFERMGMT
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void  BUFFER_MGR_InitiateSystemResources(void)
{
    shmem_data_p = (BUFFER_MGR_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_BUFFERMGMT_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);
    shmem_data_p->is_allocated=FALSE;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for BUFFERMGMT in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_AttachSystemResources(void)
{
    shmem_data_p = (BUFFER_MGR_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_BUFFERMGMT_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_BUFFER_MGR, &buffer_mgr_sem_id);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_EnterMasterMode (void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Sets to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_SetTransitionMode (void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_ProvisionComplete
 *---------------------------------------------------------------------------------
 * PURPOSE: All provision commands are settle down.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_ProvisionComplete(void)
{
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFER_MGR_Allocate
 *---------------------------------------------------------------------------------
 * FUNCTION: allocate landing buffer
 * INPUT   : none
 * OUTPUT  : none
 * RETURN  : the start address of the buffer
 * NOTE    : if fail return NULL
 *---------------------------------------------------------------------------------*/
void *BUFFER_MGR_Allocate(void)
{
    void *buffer_p;

    BUFFER_MGR_EnterCriticalSection();

    if (shmem_data_p->is_allocated)
    {
        buffer_p = NULL;
    }
    else
    {
        buffer_p = shmem_data_p->buffer;
        shmem_data_p->is_allocated = TRUE;
    }

    BUFFER_MGR_LeaveCriticalSection();

    return buffer_p;
}
/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFER_MGR_Free
 *---------------------------------------------------------------------------------
 * FUNCTION: free landing buffer
 * INPUT   : the start address of the buffer
 * OUTPUT  : None
 * RETURN  : TRUE(Success) ;FALSE
 * NOTE    :
 *---------------------------------------------------------------------------------*/
BOOL_T BUFFER_MGR_Free(void *buf_p)
{
    BOOL_T  ret = FALSE;

    BUFFER_MGR_EnterCriticalSection();

    if (!shmem_data_p->is_allocated)
    {
        ret = FALSE;
    }
    else
    {
        memset(shmem_data_p->buffer, 0, sizeof(shmem_data_p->buffer));
        shmem_data_p->is_allocated = FALSE;
        ret = TRUE;
    }

    BUFFER_MGR_LeaveCriticalSection();

    return ret;
}/* End of BUFFER_MGR_Free() */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFER_MGR_GetPtr
 *---------------------------------------------------------------------------------
 * FUNCTION: convert offset to pointer
 * INPUT   : the offset of the buffer
 * OUTPUT  : None
 * RETURN  : the pointer converted from the specified offset.
 * NOTE    :
 *---------------------------------------------------------------------------------*/
void *BUFFER_MGR_GetPtr(I32_T offset)
{
    return L_CVRT_GET_PTR(shmem_data_p, offset);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFER_MGR_GetOffset
 *---------------------------------------------------------------------------------
 * FUNCTION: convert buffer pointer to offset
 * INPUT   : the pointer of the buffer
 * OUTPUT  : None
 * RETURN  : the offset converted from the specified buffer.
 * NOTE    :
 *---------------------------------------------------------------------------------*/
I32_T BUFFER_MGR_GetOffset(void *buf_p)
{
    return L_CVRT_GET_OFFSET(shmem_data_p, buf_p);
}

