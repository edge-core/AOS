#ifndef SYS_CALLBACK_REFINED_OM_H
#define SYS_CALLBACK_REFINED_OM_H
#include "sys_cpnt.h"
#include "sys_adpt.h"


#define SYS_CALLBACK_MAX_ENTRY 4094

typedef struct
{
  UI32_T callback_event_id;
  UI32_T sub_callback_event_id;
  UI32_T src_csc_id;
  BOOL_T is_notify_ipcfail;
  SYS_CALLBACK_MGR_REFINEList_CBData_T allarg;
 }SYS_CALLBACK_ARG_ENTRY_INFO;

void SYS_CALLBACK_REFINED_OM_InitResource();
void SYS_CALLBACK_REFINED_OM_Init();

void SYS_CALLBACK_REFINED_OM_ClearDB();

UI16_T SYS_CALLBACK_REFINED_OM_GetHead();

BOOL_T SYS_CALLBACK_REFINED_OM_DeQueue(SYS_CALLBACK_ARG_ENTRY_INFO* callback_data);


BOOL_T SYS_CALLBACK_REFINED_OM_GetQueuePointedData(UI16_T position,SYS_CALLBACK_ARG_ENTRY_INFO* msg);

BOOL_T SYS_CALLBACK_REFINED_OM_SetQueuePointedData(UI16_T position,SYS_CALLBACK_ARG_ENTRY_INFO msg);

BOOL_T SYS_CALLBACK_REFINED_OM_QueueIsFull();

BOOL_T SYS_CALLBACK_REFINED_OM_QueueIsEmpty();

BOOL_T SYS_CALLBACK_REFINED_OM_EnQueue(SYS_CALLBACK_ARG_ENTRY_INFO* msg);


#endif
