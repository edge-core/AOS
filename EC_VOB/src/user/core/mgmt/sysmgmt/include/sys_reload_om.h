/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_RELOAD_OM.H				           						
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

#ifndef SYS_RELOAD_OM_H
#define SYS_RELOAD_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_reload_type.h"


/* struct */
typedef struct 
{
    UI32_T year;
    UI32_T month;
    UI32_T day;
    UI32_T hour;
    UI32_T minute;
}SYS_RELOAD_OM_RELOADAT_DST;

typedef struct
{
    UI32_T                              day_of_month;
    UI32_T                              day_of_week;
    UI32_T                              hour;
    UI32_T                              minute;
    SYS_RELOAD_TYPE_REGULARITY_PERIOD_E  period;     
}SYS_RELOAD_OM_RELOADREGULARITY_DST;

/* NAMING CONSTANT DECLARATIONS
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_Init
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system resource database
 * 																		
 * INPUT : None                                     				
 * OUTPUT: None                          					
 * RETURN: TRUE/FALSE                                               		
 * NOTES: 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_Init(void);

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
void SYS_RELOAD_OM_SetReloadIn(UI32_T minute, UI8_T *reason);

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
void SYS_RELOAD_OM_SetReloadAt(SYS_RELOAD_OM_RELOADAT_DST reload_at, UI8_T *reason);

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
void SYS_RELOAD_OM_SetReloadRegularity(SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity, UI8_T *reason);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_RELOAD_OM_GetReloadInInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will get reload-in to SYS_RELOAD_OM.
 * INPUT    : UI32_T *remain_seconds
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
void SYS_RELOAD_OM_GetReloadInInfo(UI32_T *remain_seconds);

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
void SYS_RELOAD_OM_GetReloadAtInfo(SYS_RELOAD_OM_RELOADAT_DST *reload_at);

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
void SYS_RELOAD_OM_GetReloadRegularityInfo(SYS_RELOAD_OM_RELOADREGULARITY_DST *reload_regularity);

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
void SYS_RELOAD_OM_ReloadInCancel(void);

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
void SYS_RELOAD_OM_ReloadAtCancel(void);

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
void SYS_RELOAD_OM_ReloadRegularityCancel(void);

#endif /* END OF SYS_RELOAD_OM_H */
