#include "sys_bld.h"
#include "sysfun.h"
#include "http_om_exp.h"

static UI32_T http_om_exp_semid;

BOOL_T HTTP_OM_InitateProcessResource(void)
{
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_OM, &http_om_exp_semid)!=SYSFUN_OK)
    {
        printf("%s:get om sem id fail.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}