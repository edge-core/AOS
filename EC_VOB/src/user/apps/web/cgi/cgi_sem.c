#include "sys_bld.h"
#include "sysfun.h"
#include "cgi_sem.h"

static UI32_T cgi_sem_id;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CGI_SEM_Create
 * ------------------------------------------------------------------------
 * PURPOSE  :   Create a semaphore object
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T CGI_SEM_Create(void)
{
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &cgi_sem_id) != SYSFUN_OK)
    {
        printf("%s:%d: SYSFUN_CreateSem fail\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CGI_SEM_GetId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get semaphore ID
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   Semaphore ID
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
UI32_T CGI_SEM_GetId()
{
    return cgi_sem_id;
}

