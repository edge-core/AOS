#include "msl_group.h"
#include "msl_task.h"
#include "msl_macro.h"

void 
MSL_GROUP_InitiateProcessResource(void)
{
  msl_task_global_init();
}


void 
MSL_GROUP_Create_InterCSC_Relation(void)
{
  msl_task_create_csc_relation(); 
}

void 
MSL_GROUP_Create_All_Threads(void)
{
  msl_task_create_task();
}




