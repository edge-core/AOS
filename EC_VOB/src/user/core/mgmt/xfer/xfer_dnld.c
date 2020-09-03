/* Project Name: Mercury
 * Module Name : XFER_DNLD.C
 * Abstract    :
 * Purpose     :
 *
 * History :
 *          Date        Modifier        Reason
 *          2001/10/11  BECK CHEN       Create this file
 *          2002/12/05  Erica Li        Modify callback status
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 *
 * Note    :
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "syslog_type.h"
#include "syslog_mgr.h"
#include "syslog_pmgr.h"
#include "l_mm.h"
#include "sys/socket.h"

#include "tftp.h"
#include "xfer_dnld.h"
#include "xfer_type.h"
#include "sys_module.h"
#include "l_stdlib.h"

#include "cmdftp.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define XFER_DNLD_MSG_Q_LEN   256


/* TYPE DECLARATIONS
 */

/*
 * STATIC   VARIABLES DECLARATIONS
 */
static BOOL_T is_transition_done;
/* declare variables used for Transition mode
 */
SYSFUN_DECLARE_CSC


/* LOCAL VARIABLES
 */
static UI32_T              xfer_dnld_task_id;                 /* task id */
static SYSFUN_MsgQ_T       msgq_id;
static SYS_TYPE_CallBack_T *status_callback;

/*
 * LOCAL FUNCTIONS
 */
static void		XFER_DNLD_Tftp_Main (void);
static void		XFER_DNLD_TftpDownload (XFER_DNLD_Msg_T TftpInfo);
static void 	XFER_DNLD_TftpUpload (XFER_DNLD_Msg_T TftpInfo);
static void     XFER_DNLD_FtpDownload(XFER_DNLD_Msg_T TftpInfo);
static void     XFER_DNLD_FtpUpload(XFER_DNLD_Msg_T TftpInfo);
static void     XFER_DNLD_Notify_tftpDnldStatus(UI32_T status, UI32_T download_length);
static void     XFER_DNLD_Transmitting_CallBack (UI32_T percent);

/* NAMING CONSTANT DECLARATIONS
 */
#define	XFER_DNLD_EVENT_MSG                 BIT_1
#define XFER_DNLD_EVENT_ENTER_TRANSITION    BIT_2



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_Init
 *-------------------------------------------------------------
 * FUNCTION: This function init the TFTP download function.
 *
 * INPUT   :
 * OUTPUT  :
 * RETURN  :
 * Note :
 *--------------------------------------------------------------*/
void XFER_DNLD_Init (void)
{
    is_transition_done = FALSE;
    return ;
}/* end XFER_DNLD_Init() */

/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_CreateTask
 *-------------------------------------------------------------
 * FUNCTION: This function creat the TFTP download task.
 *
 * INPUT   :
 * OUTPUT  :
 * RETURN  :
 * Note :
 *--------------------------------------------------------------*/
void XFER_DNLD_CreateTask(void)
{
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;

	if(SYSFUN_SpawnThread (SYS_BLD_TFTP_DOWNLOAD_THREAD_PRIORITY,
                          SYS_BLD_TFTP_DOWNLOAD_THREAD_SCHED_POLICY,
                          SYS_BLD_TFTP_DOWNLOAD_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          XFER_DNLD_Tftp_Main,
                          0,
                          &xfer_dnld_task_id)!=SYSFUN_OK )
    {
        owner_info.level =           SYSLOG_LEVEL_CRIT;
        owner_info.module_no =       SYS_MODULE_XFER;
        owner_info.function_no =     XFER_DNLD_FUN_CREATE_TASK;
        owner_info.error_no = XFER_DNLD_Create_Tasks_ErrNo;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, "XFER_DNLD", 0, 0);
        return;
    }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_XFER_DNLD, xfer_dnld_task_id, SYS_ADPT_XFER_SW_WATCHDOG_TIMER);
#endif

}/* end XFER_DNLD_CreateTask() */


/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_EnterMasterMode
 *-------------------------------------------------------------
 * FUNCTION: This function enter master mode.
 *
 * INPUT   :
 * OUTPUT  :
 * RETURN  :
 * Note :
 *--------------------------------------------------------------*/
void XFER_DNLD_EnterMasterMode(void)
{
   /* MGR's enter master mode, TASK also ref. to this
    */
   SYSFUN_ENTER_MASTER_MODE();

   TFTP_SetCallback(XFER_DNLD_Transmitting_CallBack);
   CMDFTP_SetCallback(XFER_DNLD_Transmitting_CallBack);
}


/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_SetTransitionMode
 *-------------------------------------------------------------
 * FUNCTION: This function set TransitionMode .
 *
 * INPUT   :
 * OUTPUT  :
 * RETURN  :
 * Note :
 *--------------------------------------------------------------*/
void XFER_DNLD_SetTransitionMode(void)
{
	/* MGR's set transition mode
	 */
	SYSFUN_SET_TRANSITION_MODE();

    /* TASK's set transition mode
     */
    is_transition_done = FALSE;
    SYSFUN_SendEvent(xfer_dnld_task_id, XFER_DNLD_EVENT_ENTER_TRANSITION);
}


/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_EnterTransitionMode
 *-------------------------------------------------------------
 * FUNCTION: This function enter TransitionMode .
 *
 * INPUT   :
 * OUTPUT  :
 * RETURN  :
 * Note :
 *--------------------------------------------------------------*/
void XFER_DNLD_EnterTransitionMode(void)
{
	/* MGR's enter transition mode
	 */
	SYSFUN_ENTER_TRANSITION_MODE();

    /* TASK's transition mode
     */
    /* Ask task to release all resources
     */
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
}


/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_EnterSlaveMode
 *-------------------------------------------------------------
 * FUNCTION: This function enter SlaveMode .
 *
 * INPUT   :
 * OUTPUT  :
 * RETURN  :
 * Note :
 *--------------------------------------------------------------*/
void XFER_DNLD_EnterSlaveMode(void)
{
    /* MGR's enter slave mode, TASK also ref. to this
     */
    SYSFUN_ENTER_SLAVE_MODE();
}


SYS_TYPE_Stacking_Mode_T XFER_DNLD_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}


/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_tftp
 *-------------------------------------------------------------
 * FUNCTION: This function create the TFTP download function.
 *
 * INPUT   :
 * OUTPUT  :
 * RETURN  :
 * Note :
 *--------------------------------------------------------------*/
BOOL_T XFER_DNLD_tftp(XFER_DNLD_Msg_T *ReceiveDnldMsg)
{
    XFER_DNLD_Msg_T     Tftp_info;
    XFER_DNLD_Mtext_T   *mtext;
    SYSFUN_Msg_T *req_msg_p;
    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_DNLD_Msg_T))];

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(XFER_DNLD_Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_DNLD_TFTP)) ) == NULL)
    {
       // DBG_Print(L_MEM_Allocate FAIL);
        return FALSE;
    }

    Tftp_info.is_download =  ReceiveDnldMsg->is_download;
    Tftp_info.server_type = ReceiveDnldMsg->server_type;
    memcpy(mtext, ReceiveDnldMsg->mtext, sizeof(XFER_DNLD_Mtext_T));

    mtext->uploadLength = ReceiveDnldMsg->mtext->uploadLength;

    Tftp_info.mtext = mtext;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_type = 1;
    req_msg_p->msg_size= sizeof(XFER_DNLD_Msg_T);
    memcpy(req_msg_p->msg_buf,&Tftp_info,sizeof(XFER_DNLD_Msg_T));

    if(SYSFUN_SendRequestMsg(msgq_id, req_msg_p,
                             SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_DNLD_EVENT_MSG, 0, NULL)!= SYSFUN_OK)
    {
        L_MM_Free(mtext);
        DBG_PrintText (" XFER_DNLD : Send event to XFER_DNLD fail..\n");
        return FALSE;
    }

    return TRUE;

} /* end XFER_DNLD_tftp() */



/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_SetCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when tftp error or tftp completed
 *           the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T trunk_ifindex, UI32_T member_ifindex)
 *
 * -------------------------------------------------------------------------*/
void XFER_DNLD_SetCallback(void (*fun)(UI32_T status, UI32_T download_length) )
{
    SYS_TYPE_REGISTER_CALLBACKFUN(status_callback);

} /* End of XFER_DNLD_SetCallback() */




/*-----------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_Abort
 *-----------------------------------------------------------------
 * FUNCTION: CLI abort tftp task
 * INPUT   :NONE
 * OUTPUT  :NONE
 * RETURN  :
 *-----------------------------------------------------------------*/
void XFER_DNLD_Abort(void)
{
  TFTP_SetProgressBar(TRUE);
} /* End of XFER_DNLD_Abort() */


/*
 * LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_Tftp_Main
 *--------------------------------------------------------------
 * FUNCTION: This function start TFTP download
 *
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : ......
 *--------------------------------------------------------------*/
static void XFER_DNLD_Tftp_Main( void)
{
    UI32_T    event_var;
    UI32_T    wait_events;
    UI32_T    rcv_events;
    UI32_T    timeout;
    UI32_T    ret_value;
    SYS_TYPE_Stacking_Mode_T got_operation_mode;

    XFER_DNLD_Msg_T TftpInfo;

    UI8_T msg_space_p[sizeof(SYSFUN_Msg_T)+sizeof(XFER_DNLD_Msg_T)];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;

    /* Prepare waiting event and init. event var. */
    wait_events = XFER_DNLD_EVENT_MSG | 
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                  SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                  XFER_DNLD_EVENT_ENTER_TRANSITION;
    event_var = 0;
    if(SYSFUN_CreateMsgQ(SYS_BLD_CSC_XFER_DNLD_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &msgq_id)
        != SYSFUN_OK)
    {
        while(1);
    }

    while(1)
    {
        /* Check timer event and message event */
        if (event_var != 0)
        {    /* There is some message in message queue  */
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
        {
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        }

        if ((ret_value=SYSFUN_ReceiveEvent (wait_events, SYSFUN_EVENT_WAIT_ANY,
                                            timeout, &rcv_events))!=SYSFUN_OK)
        {
            if (ret_value != SYSFUN_RESULT_NO_EVENT_SET)
            {
                /*    Log to system : unexpect return value    */
                ;
            }
        }
        event_var |= rcv_events;

        if (event_var==0)
        {
            /*    Log to system : ERR--Receive Event Failure */
            continue;
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_XFER_DNLD);
            event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        /* Get operation mode from MGR */
        got_operation_mode = XFER_DNLD_GetOperationMode();
        if (got_operation_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            /* task in transition mode, should clear resource (msgQ) in task */
            while( SYSFUN_ReceiveMsg( msgq_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(XFER_DNLD_Msg_T), req_msg_p)==SYSFUN_OK)
            {
                memcpy(&TftpInfo,req_msg_p->msg_buf,sizeof(XFER_DNLD_Msg_T));
                L_MM_Free ( TftpInfo.mtext );
            }    /*    end of while */

            if (event_var & XFER_DNLD_EVENT_ENTER_TRANSITION )
                is_transition_done = TRUE;

            event_var = 0;
            continue;
        }

        if (event_var & XFER_DNLD_EVENT_MSG)
        {
            if(SYSFUN_ReceiveMsg( msgq_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(XFER_DNLD_Msg_T), req_msg_p) == SYSFUN_RESULT_NO_MESSAGE)
            {
                event_var ^= XFER_DNLD_EVENT_MSG;
            }
            else /* receive a message from message queue, process the message  */
            {
                memcpy(&TftpInfo, req_msg_p->msg_buf, sizeof(TftpInfo));

                /* Remember to register XFER_DNLD_Transmitting_CallBack to
                 * each transmitting protocol on XFER_DNLD_EnterMasterMode
                 */
                switch (TftpInfo.server_type)
                {
                    case XFER_DNLD_REMOTE_SERVER_TFTP:
                        if (TftpInfo.is_download)
                        {
                            XFER_DNLD_TftpDownload(TftpInfo);
                        }
                        else
                        {
                            XFER_DNLD_TftpUpload(TftpInfo);
                        }
                        break;

#if (SYS_CPNT_SFTP == TRUE)
                    case XFER_DNLD_REMOTE_SERVER_SFTP:
                        XFER_DNLD_Sftp(TftpInfo);
                        break;
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

                    case XFER_DNLD_REMOTE_SERVER_FTP:
                    case XFER_DNLD_REMOTE_SERVER_FTPS:
                        if (TftpInfo.is_download)
                        {
                            XFER_DNLD_FtpDownload(TftpInfo);
                        }
                        else
                        {
                            XFER_DNLD_FtpUpload(TftpInfo);
                        }
                        break;

                    default:
                        printf("%s: Unhandled server type (%d)\r\n", __FUNCTION__, TftpInfo.server_type);
                        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_SUCCESS, 0);
                        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED, 0);
                }

                L_MM_Free(TftpInfo.mtext);
            }
        } /* if (event_var & XFER_DNLD_EVENT_MSG) */
    } /* End of while */
}/* end static void XFER_DNLD_Tftp_Main() */

/*----------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_TftpDownload
 *-----------------------------------------------------------------
 * FUNCTION:TFTP dowwnload (permanent download).
 *
 * INPUT   :
 * OUTPUT  :
 * RETURN  :ERROR if failed. No return if OK.
 * NOTE    :
 *
 *-----------------------------------------------------------------*/
static void XFER_DNLD_TftpDownload(XFER_DNLD_Msg_T TftpInfo)
{
	I32_T   rc;
    UI32_T  download_length = 0;
    UI32_T  error_code = 0;
	/*
	 * Open the TFTP receiver to start the file transfer.
	 */
	rc = TFTP_open(&TftpInfo.mtext->server_ip);

	if (rc)
	{
		/* return error code*/
        error_code = TFTP_GetErrorCode();
        XFER_DNLD_Notify_tftpDnldStatus(error_code,download_length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);
		return  ;
	}
	else
	{
	    rc = TFTP_get(&TftpInfo.mtext->server_ip,
                          (char *)TftpInfo.mtext->filename,
                          "octet",
                          (TftpInfo.mtext->action == XFER_DNLD_ACTION_GET_PIECE_OF_FILE)?FALSE:TRUE,
                          (char *)TftpInfo.mtext->buf,
                          TftpInfo.mtext->uploadLength,
                          TftpInfo.mtext->tftp_retry_times,
                          TftpInfo.mtext->tftp_timeout);

        download_length = rc;
        error_code = TFTP_GetErrorCode();

		if (rc == 0 && error_code == 0)/* download an empty file */
		{
            XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_SUCCESS,download_length);
            XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);

		}
		else if (rc <= 0)
		{
            XFER_DNLD_Notify_tftpDnldStatus(error_code,download_length);
            XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);

		}
		else
		{
            XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_SUCCESS,download_length);
			XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);
		}

		TFTP_close();
		return ;
	}
}/* end XFER_DNLD_TftpDownload() */




/*-----------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_TftpUpload
 *-----------------------------------------------------------------
 * FUNCTION:TFTP upload (permanent upload).
 *
 * INPUT   :XFER_DNLD_Msg_T TftpInfo
 * OUTPUT  :
 * RETURN  :ERROR if failed. No return if OK.
 * NOTE    :
 *
 *-----------------------------------------------------------------*/
static void XFER_DNLD_TftpUpload(XFER_DNLD_Msg_T TftpInfo)
{
    UI32_T  rc;
    UI32_T  download_length = 0;
    UI32_T  error_code = 0;

    /* BODY
     */
    /* Open the TFTP receiver to start the file transfer.
     */
    rc=TFTP_open(&TftpInfo.mtext->server_ip);
    if (rc)
    {
        /* return error code*/
        error_code = TFTP_GetErrorCode();
        XFER_DNLD_Notify_tftpDnldStatus(error_code,download_length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);
        return ;
    }

    rc = TFTP_put(&TftpInfo.mtext->server_ip,
                  (char *)TftpInfo.mtext->filename,
                  "octet",
                  (char *)TftpInfo.mtext->buf,
                  TftpInfo.mtext->uploadLength,
                  TftpInfo.mtext->tftp_retry_times,
                  TftpInfo.mtext->tftp_timeout);

    download_length = rc;
    error_code = TFTP_GetErrorCode();

    if (rc == 0 && error_code == 0)/* upload an empty file */
    {
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_SUCCESS,download_length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);

    }
    else if (rc <= 0)
    {
        XFER_DNLD_Notify_tftpDnldStatus(error_code,download_length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);

    }
    else
    {
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_SUCCESS,download_length);
    	XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);
    }

    TFTP_close();
    return ;
}/* end XFER_DNLD_TftpUpload() */

/*----------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_FtpDownload
 *-----------------------------------------------------------------
 * FUNCTION: FTP dowwnload (permanent download).
 * INPUT   : TftpInfo
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *
 *-----------------------------------------------------------------
 */
static void XFER_DNLD_FtpDownload(XFER_DNLD_Msg_T TftpInfo)
{
    I32_T   rc;
    UI32_T  download_length = 0;
    /*
     * Download file from FTP server
     */
    rc = CMDFTP_Transfer(   &TftpInfo.mtext->server_ip,
                            TftpInfo.mtext->username,
                            TftpInfo.mtext->password,
                            (TftpInfo.server_type == XFER_DNLD_REMOTE_SERVER_FTPS) ? TRUE : FALSE,
                            CMDFTP_TRANS_DIR_DOWNLOAD,
                            (TftpInfo.mtext->action == XFER_DNLD_ACTION_GET_PIECE_OF_FILE)?FALSE:TRUE,
                            TftpInfo.mtext->filename,
                            TftpInfo.mtext->buf,
                            TftpInfo.mtext->uploadLength,
                            &download_length);
    if (rc)
    {
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_SUCCESS,download_length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);
    }
    else
    {
        XFER_DNLD_Notify_tftpDnldStatus(CMDFTP_GetErrorCode(), download_length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,download_length);
    }

}/* end XFER_DNLD_FtpDownload() */

/*----------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_FtpUpload
 *-----------------------------------------------------------------
 * FUNCTION: FTP upload (permanent download).
 * INPUT   : TftpInfo
 *           use_ftps
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *
 *-----------------------------------------------------------------
 */
static void XFER_DNLD_FtpUpload(XFER_DNLD_Msg_T TftpInfo)
{
    I32_T   rc;
    UI32_T  upload_length = 0;
    /*
     * Download file from FTP server
     */
    rc = CMDFTP_Transfer(   &TftpInfo.mtext->server_ip,
                            TftpInfo.mtext->username,
                            TftpInfo.mtext->password,
                            (TftpInfo.server_type == XFER_DNLD_REMOTE_SERVER_FTPS) ? TRUE : FALSE,
                            CMDFTP_TRANS_DIR_UPLOAD,
                            (TftpInfo.mtext->action == XFER_DNLD_ACTION_GET_PIECE_OF_FILE)?FALSE:TRUE,
                            TftpInfo.mtext->filename,
                            TftpInfo.mtext->buf,
                            TftpInfo.mtext->uploadLength,
                            &upload_length);
    if (rc)
    {
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_SUCCESS,upload_length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,upload_length);
    }
    else
    {
        XFER_DNLD_Notify_tftpDnldStatus(CMDFTP_GetErrorCode(), upload_length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED,upload_length);
    }

}/* end XFER_DNLD_FtpUpload() */

#if (SYS_CPNT_SFTP == TRUE)
/*----------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_Sftp
 *-----------------------------------------------------------------
 * FUNCTION: FTP upload (permanent download).
 * INPUT   : TftpInfo
 *           use_ftps
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *
 *-----------------------------------------------------------------
 */
static void XFER_DNLD_Sftp(XFER_DNLD_Msg_T TftpInfo)
{
    I32_T   rc;
    UI32_T  length = 0;
    /*
     * Download file from FTP server
     */
    rc = SFTP_Main(&TftpInfo, &length);

    if (rc)
    {
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_SUCCESS, length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED, length);
    }
    else
    {
        XFER_DNLD_Notify_tftpDnldStatus(SFTP_GetErrorCode(), length);
        XFER_DNLD_Notify_tftpDnldStatus(XFER_DNLD_TFTP_COMPLETED, length);
    }
} /* XFER_DNLD_Sftp */
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_GetTftpErrorMsg
 *-------------------------------------------------------------
 * FUNCTION: Get tftp undefined message
 * INPUT   : None
 * OUTPUT  : tftp_error_msg
 * RETURN  : TRUE/FALSE
 *--------------------------------------------------------------*/
BOOL_T XFER_DNLD_GetTftpErrorMsg(UI8_T *tftp_error_msg)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return FALSE;
    }

    TFTP_GetErrorMsg((char *)tftp_error_msg);
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_Notify_tftpDnldStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void XFER_DNLD_Notify_tftpDnldStatus(UI32_T status, UI32_T download_length)
{
    SYS_TYPE_CallBack_T  *fun_list;

    for(fun_list=status_callback; fun_list; fun_list=fun_list->next)
        fun_list->func(status, download_length);
} /* End of XFER_DNLD_Notify_tftpDnldStatus() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_Transmitting_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Callback function while transmitting file.
 *           
 * INPUT   : percent - File transmitt status.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void 
XFER_DNLD_Transmitting_CallBack (
    UI32_T percent)
{
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    static UI32_T  last_wdog_reset_tick = 0;
    const UI32_T   wdog_reset_time = (SYS_ADPT_XFER_SW_WATCHDOG_TIMER * 60 * 100) / 2; /* 0.01 sec */
    UI32_T         wdog_timer = 0;

    if (percent == 0)
    {
        last_wdog_reset_tick = 0;
    }

    wdog_timer = SYSFUN_GetSysTick() - last_wdog_reset_tick;
    if (wdog_reset_time <= wdog_timer)
    {
        last_wdog_reset_tick += wdog_timer;
        SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_XFER_DNLD);
    }
#endif  /* SYS_CPNT_SW_WATCHDOG_TIMER */

    return;
}




