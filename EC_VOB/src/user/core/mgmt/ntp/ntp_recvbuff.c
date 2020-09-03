#include <string.h> /* for memset */
#include <assert.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "leaf_es3626a.h"
#include "sysfun.h"
#include "l_inet.h"
#include "sys_time.h"
#include "ntp_mgr.h"
#include "ntp_type.h"
#include "ntp_dbg.h" /* for debug use */
#include "ntp_recvbuff.h"

/*
 * Memory allocation
 */
static unsigned long volatile full_recvbufs;    /* number of recvbufs on fulllist */
static  struct recvbuf *recvbufflist= NULL;  /* recvbuff list */ /*  - QingfengZhang, 09 March, 2005 1:20:44 */
static UI32_T recv_api_semaphore; /*semaphore when operating recvbuff*/

void NTP_Recvbuff_Init(void)
{
    /* create semaphore */
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NTP_OM, &recv_api_semaphore) != SYSFUN_OK)
    {
        printf("%s:get sem id fail.\n", __FUNCTION__);
    }
}


/*
 * getrecvbufs - get receive buffers which have data in them
 *
 *
 */

struct recvbuf *NTP_RECVBUFF_GetFirstBuf()
{
    struct recvbuf * rb;

    SYSFUN_ENTER_CRITICAL_SECTION(recv_api_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);

    if(recvbufflist == NULL)
    {
        SYSFUN_LEAVE_CRITICAL_SECTION(recv_api_semaphore);
        return NULL;
    }
    else
    {
        rb = recvbufflist;
        recvbufflist = rb->next;
        SYSFUN_LEAVE_CRITICAL_SECTION(recv_api_semaphore);
        return rb;
    }
}

void NTP_RECVBUFF_AddRecvBuffer(struct recvbuf *rb)
{
    struct recvbuf *sb;

    SYSFUN_ENTER_CRITICAL_SECTION(recv_api_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);

    if (recvbufflist == NULL)
    {
        recvbufflist = rb;
    }
    else
    {
        for (sb = recvbufflist; sb->next != NULL;
             sb = sb->next) ;
        sb->next= rb;
    }

    full_recvbufs++;
    SYSFUN_LEAVE_CRITICAL_SECTION(recv_api_semaphore);
}

struct recvbuf *
NTP_RECVBUFF_GetFreeBuffer(void)
{
    struct recvbuf * buffer;

    SYSFUN_ENTER_CRITICAL_SECTION(recv_api_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    buffer =  malloc(sizeof(struct recvbuf));

    if(buffer == NULL)
    {
        printf("malloc recv buff failure! \n");
        SYSFUN_LEAVE_CRITICAL_SECTION(recv_api_semaphore);
        return NULL;
    }

    memset((char *)buffer,0,sizeof(struct recvbuf));
    SYSFUN_LEAVE_CRITICAL_SECTION(recv_api_semaphore);
    return buffer;
}

