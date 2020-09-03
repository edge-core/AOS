/* static char SccsId[] = "+-<>?!SNTP_DBG.H   22.1  05/14/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_DBG.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  05-014-2002   Created
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999
 * ------------------------------------------------------------------------
 */

#ifndef	_SNTP_DBG_H
#define	_SNTP_DBG_H
/*#define __WIN */
#define __VxWorks

#ifdef __WIN
	#include "stdio.h"
	typedef unsigned long UI32_T;
	typedef unsigned char UI16_T;
	#define BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack(str,func)	main_call_back(str,func)
	#define BACKDOOR_MGR_RequestKeyIn(index,ch_nu)	scanf("%s",&index);
	#define L_INET_Aton(ipaddress,ip)
#else
#include "sys_type.h"
#endif

extern UI32_T   DBG_SNTP_TURN_MESSAGE_ON_OFF;
void            SNTP_DBG_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_DBG_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SNTP_DBG_Create_InterCSC_Relation(void);

#endif
