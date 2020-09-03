/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_RELOAD_TYPE.H				           						
 * ------------------------------------------------------------------------
 * PURPOSE: The definitions of common data structure for SYS_RELOAD_MGR, including 
 *         command types, report types, and packet format.
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

#ifndef SYS_RELOAD_TYPE_H
#define SYS_RELOAD_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */


typedef enum
{
    SYS_RELOAD_TYPE_REGULARITY_PERIOD_NONE       = 0,
    SYS_RELOAD_TYPE_REGULARITY_PERIOD_DAILY      = 1,
    SYS_RELOAD_TYPE_REGULARITY_PERIOD_WEEKLY     = 2,
    SYS_RELOAD_TYPE_REGULARITY_PERIOD_MONTHLY    = 3
} SYS_RELOAD_TYPE_REGULARITY_PERIOD_E;
                                      
#endif /* END OF SYS_RELOAD_TYPE_H */
