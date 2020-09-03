#ifndef _DEV_NICDRV_OM_H
#define _DEV_NICDRV_OM_H

#include "sys_type.h"
#include "sysrsc_mgr.h"

void DEV_NICDRV_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

void DEV_NICDRV_OM_InitSystemResources(void);

void DEV_NICDRV_OM_AttachSystemResources(void);

BOOL_T DEV_NICDRV_OM_InitateProcessResource(void);

SYS_TYPE_Stacking_Mode_T DEV_NICDRV_OM_GetOperatingMode(void);

void DEV_NICDRV_OM_SetTransitionMode(void);

void DEV_NICDRV_OM_EnterTransitionMode(void);

void DEV_NICDRV_OM_EnterMasterMode(void);

void DEV_NICDRV_OM_EnterSlaveMode(void);

BOOL_T DEV_NICDRV_MREF_MemFree(void *buf, UI32_T cookie, void *cookie_params);

/* ----------------------------------------------------------------------------------
 * FUNCTION : DEV_NICDRV_ReserveFragmentBuffer
 * ----------------------------------------------------------------------------------
 * PURPOSE  : reserve a fragment buffer
 * INPUT    : priority   -- packet cos
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void DEV_NICDRV_ReserveFragmentBuffer(UI32_T priority);

/* ----------------------------------------------------------------------------------
 * FUNCTION : DEV_NICDRV_ReleaseFragmentBuffer
 * ----------------------------------------------------------------------------------
 * PURPOSE  : release a fragment buffer
 * INPUT    : priority   -- packet cos
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void DEV_NICDRV_ReleaseFragmentBuffer(UI32_T priority);

#endif

