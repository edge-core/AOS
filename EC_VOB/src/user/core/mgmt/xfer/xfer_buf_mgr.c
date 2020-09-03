/* Project Name: Mercury 
 * Module Name : XFER_BUF_MGR.C
 * Abstract    : 
 * Purpose     : to allocation buffer and management buffer
 *
 * History :                                                               
 *          Date              Modifier            Reason
 *          2001/10/11    BECK CHEN     Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 
 *
 * Note    : 
 */

/* INCLUDE FILE	DECLARATIONS
 */
#include <stdlib.h> 
#include <string.h>
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "xfer_buf_mgr.h"

#if (SYS_CPNT_BUFFERMGMT == TRUE)
#include "buffer_mgr.h"
#endif

/* NAMING CONSTANTS DECLARATIONS
 */
 
/*------------------------------------------------------------------------
 *                        LOCAL VARIABLES                                
 *-----------------------------------------------------------------------*/

static BOOL_T	xbuf_allocated = 0;		

#if (SYS_CPNT_BUFFERMGMT == FALSE)
static UI8_T	xbuf [SYS_ADPT_MAX_FILE_SIZE];
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_BUF_MGR_Allocate                                
 *------------------------------------------------------------------------
 * FUNCTION: This function will malloc xfer_buf                 
 * INPUT   : None                                        
 * OUTPUT  : xbuf                                                         
 * RETURN  : TRUE(Success; return xbuf); FALSE  
 * NOTE    :   
 *------------------------------------------------------------------------
 */
void *XFER_BUF_MGR_Allocate(void)
{
    void *xbuf_p;

    if (xbuf_allocated)
	{
		xbuf_p = NULL;
	}
	else
	{
#if (SYS_CPNT_BUFFERMGMT == TRUE)
        if (NULL != (xbuf_p = BUFFER_MGR_Allocate()))
        {
    	    xbuf_allocated= TRUE;
    	}
#else
        xbuf_p = xbuf;
	    xbuf_allocated= TRUE;
#endif
    }

    return xbuf_p;
}/* End of XFER_BUF_MGR_Allocate() */


/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_BUF_MGR_Allocate                                
 *------------------------------------------------------------------------
 * FUNCTION: This function will free bufffer                
 * INPUT   : void *buf_P                                                   
 * OUTPUT  : None                                                         
 * RETURN  : TRUE(Success); FALSE  
 * NOTE    :   
 *------------------------------------------------------------------------
 */
BOOL_T	XFER_BUF_MGR_Free(void *buf_P)
{
    BOOL_T ret = FALSE;
	if(xbuf_allocated )
	{
#if (SYS_CPNT_BUFFERMGMT == TRUE)
	    if (BUFFER_MGR_Free(buf_P))
	    {
	        xbuf_allocated = FALSE;
	        ret = TRUE;
	    }
#else
	    xbuf_allocated = FALSE;
	    memset(xbuf, 0, sizeof(xbuf));
        ret = TRUE;
#endif
	}/* end if */

	return ret;
}/* End of XFER_BUF_MGR_Free() */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: XFER_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for XFER MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T XFER_BUF_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    XFER_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (XFER_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (XFER_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding AMTR_MGR function
     */
    switch (msg_p->type.cmd)
    {

        case XFER_MGR_IPC_COPYFILE:
        	msg_p->type.ret_bool = XFER_MGR_CopyFile(
        	    msg_p->data.arg_grp_bool_copyfile.arg_server_ip,
        	    msg_p->data.arg_grp_bool_copyfile.arg_destfile,
        	    msg_p->data.arg_grp_bool_copyfile.arg_srcfile,
        	    msg_p->data.arg_grp_bool_copyfile.arg_file_type,
        	    msg_p->data.arg_grp_bool_copyfile.arg_mode,
        	    msg_p->data.arg_grp_bool_copyfile.arg_cookie, 
        	    msg_p->data.arg_grp_bool_copyfile.arg_ipc_message_q,        	           	            	            	            	            	    
        	    msg_p->data.arg_grp_bool_copyfile.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_copyfile);
            break;
                    
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of XFER_MGR_HandleIPCReqMsg */

