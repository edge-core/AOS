/* MODULE NAME:  cplddrv.h
 * PURPOSE:
 *     header file for declarations of functions to cplddrv
 *
 * NOTES:
 *
 * REASON:
 *    9/06/2013 - Vic Chang, Created
 *
 * Copyright(C)      Accton Corporation, 2013
 */
#ifndef	CPLDDRV_H
#define	CPLDDRV_H
/*--------------------------------------------------------------------------
 * ROUTINE NAME - CPLDDRV_Upgrade_CPLD
 *---------------------------------------------------------------------------
 * PURPOSE:  Do upgrade CPLD fw
 * INPUT:    buf  : cpld data
             bufsize: cpld data of length
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
*/

BOOL_T CPLDDRV_Upgrade_CPLD(UI8_T *buf, UI32_T bufsize);

#endif /* CPLDDRV_H */
