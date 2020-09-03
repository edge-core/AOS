/* static char SccsId[] = "+-<>?!SNTP_DBG.H   22.1  05/14/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _NTP_DBG.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *   HardSun, 2005 02 17 10:59     Created
 * ------------------------------------------------------------------------
 *  Copyright(C)        Accton Corporation, 2005
 * ------------------------------------------------------------------------
 */

#ifndef _NTP_DBG_H
#define _NTP_DBG_H
/*#define __WIN */
#define __VxWorks

#ifdef __WIN
  #include "stdio.h"
  typedef unsigned long UI32_T;
  typedef unsigned char UI16_T;
  #define BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack(str,func) main_call_back(str,func)
  #define BACKDOOR_MGR_RequestKeyIn(index,ch_nu)  scanf("%s",&index);
  #define L_INET_Aton(ipaddress,ip)
#else
#include "sys_type.h"
#endif


/*Use in backdoor for input server ip address*/
void NTP_DBG_RequestKeyIn(UI8_T *key_in_string, UI32_T max_key_len);
/*------------------------------------------------------------------------------
  * FUNCTION NAME - DBG_NTP_BackdoorInfo_CallBack
  *------------------------------------------------------------------------------
  * PURPOSE  : Backdoor call back function
  * INPUT    :
  * OUTPUT   :
  * RETURN   :
  *
  * NOTES    : backdoor use
  *------------------------------------------------------------------------------*/
 void DBG_NTP_BackdoorInfo_CallBack(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NTP_DBG_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void NTP_DBG_Create_InterCSC_Relation(void);


#endif
