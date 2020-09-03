#include "sys_cpnt.h"
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)

#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "sys_callback_mgr.h"
#include "sys_callback_om.h"
#include "sys_callback_om_private.h"
#include "sysrsc_mgr.h"
#include "sys_callback_refined_om.h"


typedef struct 
{
    SYS_CALLBACK_ARG_ENTRY_INFO entries[SYS_CALLBACK_MAX_ENTRY];
    UI16_T head;
    UI16_T tail;
} SYS_CALLBACK_Refined_OM_Data_T;

static SYS_CALLBACK_Refined_OM_Data_T   om_data_p;
static UI32_T             syscallback_refined_om_sem_id;
#define SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(syscallback_refined_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(syscallback_refined_om_sem_id)

//static void SYS_CALLBACK_REFINED_OM_AdjustQueue();
static BOOL_T SYS_CALLBACK_REFINED_OM_IsVaildPosition(UI16_T position);

void SYS_CALLBACK_REFINED_OM_InitResource()
{
     if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYS_CALLBACK_OM, &syscallback_refined_om_sem_id) != SYSFUN_OK)
    {
        printf("\n%s:get om sem id fail.\n", __FUNCTION__);
    }
}
void SYS_CALLBACK_REFINED_OM_Init()
{
    SYS_CALLBACK_REFINED_OM_ClearDB();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_REFINED_OM_ClearDB
 *-----------------------------------------------------------------------------
 * PURPOSE : clear database
 * INPUT   : NONE
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */ 

void SYS_CALLBACK_REFINED_OM_ClearDB()
{
    SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
     memset(&om_data_p,0,sizeof(SYS_CALLBACK_Refined_OM_Data_T));
     om_data_p.head=om_data_p.tail=0;
    SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();

}
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_REFINED_OM_DeQueue
 *-----------------------------------------------------------------------------
 * PURPOSE : dequeue a entry
 * INPUT   :   callback_data        entry data
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */

BOOL_T SYS_CALLBACK_REFINED_OM_DeQueue(SYS_CALLBACK_ARG_ENTRY_INFO* callback_data)
{
   if(callback_data == NULL)
    return FALSE;

   if(SYS_CALLBACK_REFINED_OM_QueueIsEmpty())
    return FALSE;
    
   SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
   memcpy(callback_data,&(om_data_p.entries[om_data_p.head]),sizeof(SYS_CALLBACK_ARG_ENTRY_INFO));
   om_data_p.head = (om_data_p.head+1)%SYS_CALLBACK_MAX_ENTRY;
  
   SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
   return TRUE;
   
}

UI16_T SYS_CALLBACK_REFINED_OM_GetHead()
{
    UI16_T head;

    SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
    head = om_data_p.head;
    SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
    return head;

}


BOOL_T SYS_CALLBACK_REFINED_OM_GetQueuePointedData(UI16_T position,SYS_CALLBACK_ARG_ENTRY_INFO* msg)
{

   if(msg == NULL)
    return FALSE;

   if(!SYS_CALLBACK_REFINED_OM_IsVaildPosition(position))
    return FALSE;

   SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
   memcpy(msg,&(om_data_p.entries[position]),sizeof(SYS_CALLBACK_ARG_ENTRY_INFO)); 
   SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
   return TRUE;
}
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_REFINED_OM_SetQueuePointedData
 *-----------------------------------------------------------------------------
 * PURPOSE : change a entry info
 * INPUT   :position   entry place
                    msg         new info data
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */

BOOL_T SYS_CALLBACK_REFINED_OM_SetQueuePointedData(UI16_T position,SYS_CALLBACK_ARG_ENTRY_INFO msg)
{


   if(!SYS_CALLBACK_REFINED_OM_IsVaildPosition(position))
    return FALSE;

   SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
   memcpy(&(om_data_p.entries[position]),&msg,sizeof(SYS_CALLBACK_ARG_ENTRY_INFO)); 
   SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
   return TRUE;
  
}
static BOOL_T SYS_CALLBACK_REFINED_OM_IsVaildPosition(UI16_T position)
{
   if(position >= SYS_CALLBACK_MAX_ENTRY)
    return FALSE;

   if(SYS_CALLBACK_REFINED_OM_QueueIsEmpty())
    return FALSE;
    
   SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
   
   if(position < om_data_p.head && om_data_p.head <=om_data_p.tail )
   {
    SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
   }

   if(position >= om_data_p.tail && om_data_p.head <=om_data_p.tail )
   {
    SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
   }

   if(position >= om_data_p.tail && position <om_data_p.head &&
      om_data_p.tail<=om_data_p.head )
   {
    SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
   }
    
   SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();

   return TRUE;

}
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_REFINED_OM_QueueIsFull
 *-----------------------------------------------------------------------------
 * PURPOSE : Check queue is full or not
 * INPUT   : NONE
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */ 

BOOL_T SYS_CALLBACK_REFINED_OM_QueueIsFull()
{
    BOOL_T ret;
      SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
       if(om_data_p.head == ((om_data_p.tail+1)%SYS_CALLBACK_MAX_ENTRY))
        ret = TRUE;
       else
        ret = FALSE;
      SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
      return ret;

}
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_REFINED_OM_QueueIsEmpty
 *-----------------------------------------------------------------------------
 * PURPOSE : Check queue is empty or not
 * INPUT   : NONE
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */ 

BOOL_T SYS_CALLBACK_REFINED_OM_QueueIsEmpty()
{
  BOOL_T ret;
  SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
   if(om_data_p.head == om_data_p.tail)
    ret = TRUE;
   else
    ret = FALSE;
  SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();
  return ret;
}
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_REFINED_OM_EnQueue
 *-----------------------------------------------------------------------------
 * PURPOSE : add a entry
 * INPUT   : SYS_CALLBACK_ARG_ENTRY_INFO* msg  msg info
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */

BOOL_T SYS_CALLBACK_REFINED_OM_EnQueue(SYS_CALLBACK_ARG_ENTRY_INFO* msg)
{
    if(msg == NULL)
     return FALSE;
    
    if(SYS_CALLBACK_REFINED_OM_QueueIsFull())
     return FALSE;

    SYS_CALLBACK_REFINED_OM_ENTER_CRITICAL_SECTION();
    memcpy(&(om_data_p.entries[om_data_p.tail]),msg,sizeof(SYS_CALLBACK_ARG_ENTRY_INFO));
    om_data_p.tail=(om_data_p.tail+1)%SYS_CALLBACK_MAX_ENTRY;    
    SYS_CALLBACK_REFINED_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
   
}
#endif

