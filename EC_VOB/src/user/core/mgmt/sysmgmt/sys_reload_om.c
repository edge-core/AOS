/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_RELOAD_OM.C				           						
 * ------------------------------------------------------------------------
 * PURPOSE: The Reload OM provides the services for
 *          Reload mgr to read/write database.
 *
 * Notes: 
 *
 *  History
 *
 *   Andy_Chang     12/24/2007      new created
 * 
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2007      
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <memory.h>
#include "sysfun.h"
#include "sys_reload_om.h"
/* NAMING CONSTANT DECLARATIONS
 */
#define SYS_RELOAD_OM_ENTER_CRITICAL_SECTION() \
    sys_reload_om_orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(sys_reload_om_sem_id);
#define SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION()  \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(sys_reload_om_sem_id, sys_reload_om_orig_priority);
 
/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
*/ 
static BOOL_T SYS_RELOAD_OM_InitSemaphore(void);

/* STATIC VARIABLE DECLARATIONS 
 */ 
static  UI32_T                              system_reload_in_time; 
static  SYS_RELOAD_OM_RELOADAT_DST          system_reload_at_time;  
static  SYS_RELOAD_OM_RELOADREGULARITY_DST  system_reload_regularity_time; 
static  UI32_T                              sys_reload_om_sem_id;
static  UI32_T                              sys_reload_om_orig_priority;

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_Init
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will init the system resource database
 * 																		
 * INPUT    : None                                     				
 * OUTPUT   : None                          					
 * RETURN   : TRUE/FALSE                                               		
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_Init(void)
{
    SYS_RELOAD_OM_InitSemaphore();

    system_reload_in_time = 0xFFFFFFFF;
    memset(&system_reload_at_time, 0, sizeof(SYS_RELOAD_OM_RELOADAT_DST));
    memset(&system_reload_regularity_time, 0, sizeof(SYS_RELOAD_OM_RELOADREGULARITY_DST));            
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_RELOAD_OM_CreatSem
 *---------------------------------------------------------------------------
 * PURPOSE:  Initiate the semaphore for SYS_RELOAD objects.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------*/
static BOOL_T SYS_RELOAD_OM_InitSemaphore(void)
{
    /* create semaphore */
    if(SYSFUN_OK!=SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &sys_reload_om_sem_id))
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_SetReloadIn
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will set reload-in to SYS_RELOAD_OM.
 * INPUT    : UI32_T minute
 *            UI8_T *reason 1-255 long
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm [text]
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_SetReloadIn(UI32_T minute, UI8_T *reason)
{    
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    system_reload_in_time = minute;
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION();        
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_SetReloadAt
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will set reload-at to SYS_RELOAD_OM.
 * INPUT    : UI32_T minute
 *            UI8_T *reason 1-255 long
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm [text]
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_SetReloadAt(SYS_RELOAD_OM_RELOADAT_DST reload_at, UI8_T *reason)
{
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    memcpy(&system_reload_at_time, &reload_at, sizeof(SYS_RELOAD_OM_RELOADAT_DST));
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION();    
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_SetReloadRegularity
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will set reload-regularity to SYS_RELOAD_OM.
 * INPUT    : UI32_T minute
 *            UI8_T *reason 1-255 long
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : cli command - reload in [hh]:mm [text]
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_SetReloadRegularity(SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity, UI8_T *reason)
{
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    memcpy(&system_reload_regularity_time, &reload_regularity, sizeof(SYS_RELOAD_OM_RELOADREGULARITY_DST));
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION();    
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_GetReloadInInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will get reload-in to SYS_RELOAD_OM. But 
 *            Upper layer should get remaining_reload_in_time in SYS_RELOAD_MGR.C
 * INPUT    : UI32_T *remain_seconds
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_GetReloadInInfo(UI32_T *remain_seconds)
{
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    *remain_seconds = system_reload_in_time*60;
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION(); 
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_GetReloadAtInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will get reload-at to SYS_RELOAD_OM.
 * INPUT    : SYS_RELOAD_OM_RELOADAT_DST *reload_at
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_GetReloadAtInfo(SYS_RELOAD_OM_RELOADAT_DST *reload_at)
{
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    memcpy(reload_at, &system_reload_at_time, sizeof(SYS_RELOAD_OM_RELOADAT_DST));
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION(); 
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_GetReloadRegularityInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will get reload-regularity to SYS_RELOAD_OM.
 * INPUT    : SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_GetReloadRegularityInfo(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity)
{
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    memcpy(reload_regularity, &system_reload_regularity_time, sizeof(SYS_RELOAD_OM_RELOADREGULARITY_DST));
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_ReloadInCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will clear reload-in to SYS_RELOAD_OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_ReloadInCancel(void)
{
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    system_reload_in_time = 0xFFFFFFFF;
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION(); 
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_ReloadInCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will clear reload-at to SYS_RELOAD_OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_ReloadAtCancel(void)
{
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    memset(&system_reload_at_time, 0, sizeof(SYS_RELOAD_OM_RELOADAT_DST));
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION(); 
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_ReloadRegularityCancel
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will clear reload-regularity to SYS_RELOAD_OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_ReloadRegularityCancel(void)
{
    SYS_RELOAD_OM_ENTER_CRITICAL_SECTION();
    memset(&system_reload_regularity_time, 0, sizeof(SYS_RELOAD_OM_RELOADREGULARITY_DST));
    SYS_RELOAD_OM_LEAVE_CRITICAL_SECTION(); 
}

