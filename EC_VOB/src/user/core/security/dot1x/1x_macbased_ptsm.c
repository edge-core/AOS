#include "1x_common.h"
//#include "1x_macbased_ptsm.h"
#include "1x_types.h"
#include "1x_auth_pae.h"
#include "1x_om.h"
#include "sysfun.h"

static I32_T /*ptsm_aWhile,*/ptsm_quietWhile[DOT1X_MAX_PORT],ptsm_txWhile[DOT1X_MAX_PORT],ptsm_supplicantWhile[DOT1X_MAX_PORT],ptsm_serverWhile[DOT1X_MAX_PORT],ptsm_reauthWhile[DOT1X_MAX_PORT];
static UI8_T ptsm_timeout_flag[DOT1X_MAX_PORT];

static void DOT1XMAC_Ptsm_Tick(UI32_T lport);
static BOOL_T DOT1XMAC_Ptsm_Check(UI32_T lport);
static BOOL_T DOT1XMAC_Ptsm_ProcessTimeout(UI32_T lport);
static BOOL_T DOT1XMAC_Ptsm_Idle(UI32_T lport);
static BOOL_T DOT1XMAC_Ptsm_TurnOnTimer(UI32_T lport,UI8_T timer_type);

void DOT1XMAC_Ptsm_RunStateMachine(UI32_T lport)
{
    //UI32_T lport;
      
    //for(lport=1;lport<=DOT1X_MAX_PORT;lport++)
    //{
          DOT1XMAC_Ptsm_Tick(lport);
          if(DOT1XMAC_Ptsm_Check(lport) == TRUE)
              DOT1XMAC_Ptsm_ProcessTimeout(lport);
          DOT1XMAC_Ptsm_Idle(lport);
     //}
     return;       
}

void DOT1XMAC_Ptsm_InitTick()
{
    UI32_T lport;
    
    for(lport=1;lport<=DOT1X_MAX_PORT;lport++)
    {
         //DOT1XMAC_OM_Get_AuthSuppTimeout(&period);
         //ptsm_aWhile = period;

          ptsm_quietWhile[lport-1] = DOT1X_OM_Get_PortQuietPeriod(lport);
                
          ptsm_txWhile[lport-1] = DOT1X_OM_Get_PortTxPeriod(lport);
            
          ptsm_supplicantWhile[lport-1] = DOT1X_OM_Get_AuthSuppTimeout(lport);
          
          ptsm_serverWhile[lport-1] = DOT1X_OM_Get_AuthServerTimeout(lport);  
            
          ptsm_reauthWhile[lport-1] = DOT1X_OM_Get_PortReAuthPeriod(lport);
             
          ptsm_timeout_flag[lport-1] = 0;
    }             
    return;
}
 
static void DOT1XMAC_Ptsm_Tick(UI32_T lport)
{
      int   quietWhile,reAuthWhen,txWhen,aWhile;    

      quietWhile = DOT1X_OM_Get_PortTimer(lport,DOT1X_OM_TIMER_QUIETWHILE);
            
      reAuthWhen = DOT1X_OM_Get_PortTimer(lport,DOT1X_OM_TIMER_REAUTHWHEN);
      
      txWhen = DOT1X_OM_Get_PortTimer(lport,DOT1X_OM_TIMER_TXWHEN);
      
      aWhile = DOT1X_OM_Get_PortTimer(lport,DOT1X_OM_TIMER_AWHILE);

      ptsm_supplicantWhile[lport-1] = ptsm_supplicantWhile[lport-1];            

      if(quietWhile >= 0)
      {
            quietWhile --;
            DOT1X_OM_Set_PortTimer(lport,quietWhile,DOT1X_OM_TIMER_QUIETWHILE);
      }
      
      if(txWhen >= 0)
      {
           txWhen --;
           DOT1X_OM_Set_PortTimer(lport,txWhen,DOT1X_OM_TIMER_TXWHEN);
      }
      
      if(ptsm_supplicantWhile[lport-1] >= 0)
      {
           ptsm_supplicantWhile[lport-1] --;
           ptsm_supplicantWhile[lport-1] = ptsm_supplicantWhile[lport-1];
      }

      if(ptsm_serverWhile[lport-1] >= 0)
      {
          ptsm_serverWhile[lport-1] --;
      }
        
      if(reAuthWhen >= 0)
      {
           reAuthWhen--;
           DOT1X_OM_Set_PortTimer(lport,reAuthWhen,DOT1X_OM_TIMER_REAUTHWHEN);
      }   
      return;
}

static BOOL_T DOT1XMAC_Ptsm_Check(UI32_T lport)
{
     BOOL_T timeout = FALSE ;
#if 0 /* JinhuaWei, 05 August, 2008 12:36:40 */
     UI32_T period;
#endif /* #if 0 */
     int   quietWhile,reAuthWhen,txWhen,aWhile;
      
     quietWhile = DOT1X_OM_Get_PortTimer(lport,DOT1X_OM_TIMER_QUIETWHILE);
     reAuthWhen = DOT1X_OM_Get_PortTimer(lport,DOT1X_OM_TIMER_REAUTHWHEN);
     txWhen = DOT1X_OM_Get_PortTimer(lport,DOT1X_OM_TIMER_TXWHEN);
     aWhile = DOT1X_OM_Get_PortTimer(lport,DOT1X_OM_TIMER_AWHILE);

//   if(ptsm_aWhile <= 0)
//   {
//        timeout = DOT1XMAC_Ptsm_TurnOnTimer(DOT1XMAC_TIMER_TYPE_AWHILE);
//        DOT1XMAC_OM_Get_AuthSuppTimeout(&period);
//        ptsm_aWhile = period;
//   }
     if(quietWhile <= 0)
     {
           timeout = DOT1XMAC_Ptsm_TurnOnTimer(lport,DOT1XMAC_TIMER_TYPE_QUIETWHILE);
           //DOT1XMAC_OM_Get_QuietPeriod(&period);
           //ptsm_quietWhile[lport-1] = DOT1X_OM_Get_PortQuietPeriod(lport);   
           DOT1X_OM_Set_PortTimer(lport,DOT1X_OM_Get_PortQuietPeriod(lport),DOT1X_OM_TIMER_QUIETWHILE);
     }
      
     if(txWhen <= 0)
     {
           timeout = DOT1XMAC_Ptsm_TurnOnTimer(lport,DOT1XMAC_TIMER_TYPE_TXWHILE);
           //DOT1XMAC_OM_Get_TxPeriod(&period);
           //ptsm_txWhile[lport-1] = DOT1X_OM_Get_PortTxPeriod(lport);
           DOT1X_OM_Set_PortTimer(lport,DOT1X_OM_Get_PortTxPeriod(lport),DOT1X_OM_TIMER_TXWHEN);
     }
      
     if(ptsm_supplicantWhile[lport-1] <= 0)
     {
            timeout = DOT1XMAC_Ptsm_TurnOnTimer(lport,DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE); 
            //DOT1XMAC_OM_Get_AuthSuppTimeout(&period);
            ptsm_supplicantWhile[lport-1] = DOT1X_OM_Get_AuthSuppTimeout(lport);    
     }
      
     if(ptsm_serverWhile[lport-1] <= 0)
     {
            timeout = DOT1XMAC_Ptsm_TurnOnTimer(lport,DOT1XMAC_TIMER_TYPE_SERVERWHILE);
            //DOT1XMAC_OM_Get_AuthServerTimeout(&period); 
            ptsm_serverWhile[lport-1] = DOT1X_OM_Get_AuthServerTimeout(lport);
      }
      
      if(reAuthWhen <= 0)
      {
            timeout = DOT1XMAC_Ptsm_TurnOnTimer(lport,DOT1XMAC_TIMER_TYPE_REAUTHWHILE);
            //DOT1XMAC_OM_Get_ReAuthPeriod(&period);
            //ptsm_reauthWhile[lport-1] = DOT1X_OM_Get_PortReAuthPeriod(lport);
            DOT1X_OM_Set_PortTimer(lport,DOT1X_OM_Get_PortReAuthPeriod(lport),DOT1X_OM_TIMER_REAUTHWHEN);
      }
                    
      return timeout;
 }
 
static BOOL_T DOT1XMAC_Ptsm_ProcessTimeout(UI32_T lport)
{
      UI16_T mac_entry_index;
      DOT1X_MacEntry_T *mac_entry_ptr=NULL;
     
      for(mac_entry_index=0;mac_entry_index<DOT1XMAC_TOTAL_MAC_TABLE;mac_entry_index++)
      {
            mac_entry_ptr = DOT1X_OM_Get_MacEntry(mac_entry_index);
            if((mac_entry_ptr -> used_flag == TRUE) && (mac_entry_ptr ->lport == lport))
            {
                  //SYSFUN_Sleep (5);
                  DOT1XMAC_VM_Do_Authenticator(mac_entry_ptr -> mac_entry_gp,DOT1X_PTSM_TIMEOUT);
            }
      } 
      return TRUE;
}

static BOOL_T DOT1XMAC_Ptsm_Idle(UI32_T lport)
{
      ptsm_timeout_flag[lport-1] = 0;
      return TRUE;
}

static BOOL_T DOT1XMAC_Ptsm_TurnOnTimer(UI32_T lport,UI8_T timer_type)
{
     BOOL_T timeout = FALSE ;
    
//  if(timer_type == DOT1XMAC_TIMER_TYPE_AWHILE)
//      {
//          ptsm_timeout_flag = ptsm_timeout_flag | DOT1XMAC_TIMER_EVENT_AWHILE ;
//          timeout = TRUE;
//      } 
     if(timer_type == DOT1XMAC_TIMER_TYPE_QUIETWHILE)
     {
           ptsm_timeout_flag[lport-1] = ptsm_timeout_flag[lport-1] | DOT1XMAC_TIMER_EVENT_QUIETWHILE ; 
           timeout = TRUE;
     }
     if(timer_type == DOT1XMAC_TIMER_TYPE_TXWHILE)
     {
           ptsm_timeout_flag[lport-1] = ptsm_timeout_flag[lport-1] | DOT1XMAC_TIMER_EVENT_TXWHILE ;
           timeout = TRUE;
     }
     if(timer_type == DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE)
     {
          ptsm_timeout_flag[lport-1] = ptsm_timeout_flag[lport-1] | DOT1XMAC_TIMER_EVENT_SUPPLICANTWHILE ;
          timeout = TRUE;
     }
     if(timer_type == DOT1XMAC_TIMER_TYPE_SERVERWHILE)
     {
           ptsm_timeout_flag[lport-1] = ptsm_timeout_flag[lport-1] | DOT1XMAC_TIMER_EVENT_SERVERWHILE ;
           timeout = TRUE;
     }
     if(timer_type == DOT1XMAC_TIMER_TYPE_REAUTHWHILE)
     {
           ptsm_timeout_flag[lport-1] = ptsm_timeout_flag[lport-1] | DOT1XMAC_TIMER_EVENT_REAUTHWHILE ;
           timeout = TRUE;
     }
     return timeout;   
}

BOOL_T DOT1XMAC_Ptsm_SetTimer(Global_Params *global,UI8_T timer_type,UI8_T flag)
{
    DOT1X_MacEntry_T *mac_entry_ptr=NULL; 
    mac_entry_ptr = DOT1X_OM_Get_MacEntry(global -> mac_entry_index);
//  if(timer_type == DOT1XMAC_TIMER_TYPE_AWHILE)
//      {
//          if(flag == 1)
//              mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive | DOT1XMAC_TIMER_EVENT_AWHILE ;
//          else
//              mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive & (!DOT1XMAC_TIMER_EVENT_AWHILE) ;
//      } 
      if(timer_type == DOT1XMAC_TIMER_TYPE_QUIETWHILE)
      {
            if(flag == 1)
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive | DOT1XMAC_TIMER_EVENT_QUIETWHILE ;
            else
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive & (!DOT1XMAC_TIMER_EVENT_QUIETWHILE) ;
      }
      if(timer_type == DOT1XMAC_TIMER_TYPE_TXWHILE)
      {
            if(flag == 1)
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive | DOT1XMAC_TIMER_EVENT_TXWHILE ;
            else
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive & (!DOT1XMAC_TIMER_EVENT_TXWHILE) ;
      }
      if(timer_type == DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE)
      {
            if(flag == 1)
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive | DOT1XMAC_TIMER_EVENT_SUPPLICANTWHILE ;
            else
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive & (!DOT1XMAC_TIMER_EVENT_SUPPLICANTWHILE) ;
      }
      if(timer_type == DOT1XMAC_TIMER_TYPE_SERVERWHILE)
      {
            if(flag == 1)
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive | DOT1XMAC_TIMER_EVENT_SERVERWHILE ;
            else
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive & (!DOT1XMAC_TIMER_EVENT_SERVERWHILE) ;
      }
      if(timer_type == DOT1XMAC_TIMER_TYPE_REAUTHWHILE)
      {
            if(flag == 1)
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive | DOT1XMAC_TIMER_EVENT_REAUTHWHILE ;
            else
                  mac_entry_ptr -> timeractive = mac_entry_ptr -> timeractive &(!DOT1XMAC_TIMER_EVENT_REAUTHWHILE) ;
      }
      return TRUE;  
}

BOOL_T DOT1XMAC_Ptsm_GetTimer(Global_Params *global,UI8_T timer_type,UI8_T *flag)
{
    DOT1X_MacEntry_T *mac_entry_ptr=NULL; 
    
    
    mac_entry_ptr = DOT1X_OM_Get_MacEntry(global -> mac_entry_index);
        
//  if(timer_type == DOT1XMAC_TIMER_TYPE_AWHILE)
//      *flag = mac_entry_ptr -> timeractive & DOT1XMAC_TIMER_EVENT_AWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_QUIETWHILE)
        *flag = mac_entry_ptr -> timeractive & DOT1XMAC_TIMER_EVENT_QUIETWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_TXWHILE)
        *flag = mac_entry_ptr -> timeractive & DOT1XMAC_TIMER_EVENT_TXWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE)
        *flag = mac_entry_ptr -> timeractive & DOT1XMAC_TIMER_EVENT_SUPPLICANTWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_SERVERWHILE)
        *flag = mac_entry_ptr -> timeractive & DOT1XMAC_TIMER_EVENT_SERVERWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_REAUTHWHILE)
        *flag = mac_entry_ptr -> timeractive & DOT1XMAC_TIMER_EVENT_REAUTHWHILE ;
      return TRUE;  
}

 BOOL_T DOT1XMAC_Ptsm_IsTimeout(UI32_T lport,UI8_T timer_type)
{
    UI8_T result = 0;
    BOOL_T timeout = FALSE;
    
//  if(timer_type == DOT1XMAC_TIMER_TYPE_AWHILE)
//      result = ptsm_timeout_flag & DOT1XMAC_TIMER_EVENT_AWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_QUIETWHILE)
        result = ptsm_timeout_flag[lport-1] & DOT1XMAC_TIMER_EVENT_QUIETWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_TXWHILE)
        result = ptsm_timeout_flag[lport-1] & DOT1XMAC_TIMER_EVENT_TXWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE)
        result = ptsm_timeout_flag[lport-1] & DOT1XMAC_TIMER_EVENT_SUPPLICANTWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_SERVERWHILE)
        result = ptsm_timeout_flag[lport-1] & DOT1XMAC_TIMER_EVENT_SERVERWHILE ;
    if(timer_type == DOT1XMAC_TIMER_TYPE_REAUTHWHILE)
        result = ptsm_timeout_flag[lport-1] & DOT1XMAC_TIMER_EVENT_REAUTHWHILE ;
    if(result > 0)
        timeout = TRUE; 
      return timeout;   
}

void DOT1XMAC_Ptsm_ResetTimer(UI32_T lport,UI8_T timer_type,UI32_T seconds) 
{
//   if(timer_type == DOT1XMAC_TIMER_TYPE_AWHILE)
//   {
//          ptsm_aWhile = seconds;
//   } 
     if(timer_type == DOT1XMAC_TIMER_TYPE_QUIETWHILE)
     {
          ptsm_quietWhile[lport-1] = seconds;
     }
     if(timer_type == DOT1XMAC_TIMER_TYPE_TXWHILE)
     {
          ptsm_txWhile[lport-1] = seconds;
     }
     if(timer_type == DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE)
     {
           ptsm_supplicantWhile[lport-1] = seconds;
     }
     if(timer_type == DOT1XMAC_TIMER_TYPE_SERVERWHILE)
     {
          ptsm_serverWhile[lport-1] = seconds;
     }
     if(timer_type == DOT1XMAC_TIMER_TYPE_REAUTHWHILE)
     {
           ptsm_reauthWhile[lport-1] = seconds;
     }
     return;
}
