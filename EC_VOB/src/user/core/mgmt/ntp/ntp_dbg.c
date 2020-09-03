/* static char SccsId[] = "+-<>?!NTP_DBG.C   22.1  05/14/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _NTP_DBG.C
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  HardSun, 2005 02 17 10:59     Created
 * ------------------------------------------------------------------------
 *  Copyright(C)        Accton Corporation, 2005
 * ------------------------------------------------------------------------
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include "backdoor_mgr.h"
 #include "l_inet.h"
 #include "app_protocol_proc_comm.h"
 #include "sys_time.h"
 #include "ntp_type.h"
 #include "ntp_task.h"
 #include "ntp_mgr.h"
 #include "ntp_recvbuff.h"
 #include "ntp_dbg.h"


/* Global variable for printing debug message*/

/*debug use */

/*extern void NTP_OM_GetIpTable(UI32_T *server_entry,NTP_SERVER_STATUS_E *server_status);
*/
/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NTP_DBG_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void NTP_DBG_Create_InterCSC_Relation(void)
{
    /* Back door function : regerister to cli */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("NTP",
                                                       SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY,
                                                       DBG_NTP_BackdoorInfo_CallBack);
}

 /* Back door function
  */
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
void DBG_NTP_BackdoorInfo_CallBack(void)
{
     UI8_T ch;

     BACKDOOR_MGR_Printf("\nNTP_MGR: NTP Backdoor Selection");

     while(1)
     {
         BACKDOOR_MGR_Printf("\n\tC : close show debug message ");
         BACKDOOR_MGR_Printf("\n\tD : show debug message ");
         BACKDOOR_MGR_Printf("\n\tS : show all server delay ");
         BACKDOOR_MGR_Printf("\n\tX : Return \n");
         ch = getchar();

         switch(ch)
         {
             case 'C' :
                 NTP_MGR_Debug_Disable();
                 break;
             case 'D' :
                 NTP_MGR_Debug_Enable();
                 break;
             case 'S' :
                 NTP_MGR_Show_Delay();
                 break;

             case 'x':
             case 'X':
                 return;
         }
     }
}

#if 0
/*Use in backdoor for input server ip address*/
void NTP_DBG_RequestKeyIn(UI8_T *key_in_string, UI32_T max_key_len)
{
    int character;
    int count = 0;

    while(1)
    {
        character = getchar();

        if(character == EOF || character == '\n' || character == 0x0d)     /*newline*/
        {                                         /*pttch add 2002.08.27 if ioctl set to raw mode*/
            *(key_in_string + count) = 0;         /*enter will not send with new line so check it*/
            return;
        }

        if( character >=  0x20 &&  character <= 0x7E) /*printable*/
        {
            if(count + 1 <= max_key_len)
            {
               *(key_in_string + count) = (UI8_T)character;
                BACKDOOR_MGR_Printf("%c", character);
                count++;
            }
        }
        else if(character == 0x08 && count > 0)        /*backspace*/
        {
            count --;
            BACKDOOR_MGR_Printf("\x08\x20\x08"); /*perform backspace on screen*/
        }
    }
    return;
}
#endif


