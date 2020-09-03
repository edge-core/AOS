/* Project Name: Mercury 
 * Module Name : XFER_DNLD.h
 * Abstract    : 
 * Purpose     : 
 *
 * History :                                                               
 *          Date        Modifier        Reason
 *          2001/10/11  BECK CHEN       Create this file
 *          2002/12/05  Erica Li        Add XFER_DNLD_TFTPStatus_T
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 
 *
 * Note    : 
 */

#ifndef _XFER_DNLD_H_
#define _XFER_DNLD_H_


/*
 *INCLUDE STRUCTURES                             
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "leaf_es3626a.h"
#include "l_inet.h"
#include "xfer_type.h"

/*
 * NAME CONSTANT DECLARATIONS 
 */
#define XFER_DNLD_Create_Tasks_ErrNo 0
#define XFER_DNLD_DFLT_FTP_PORT 21

enum XFER_DNLD_FUN_E
{
    XFER_DNLD_FUN_CREATE_TASK = 0,
    XFER_DNLD_FUN_TFTP
};

/* TYPE DECLARATIONS
 */
typedef enum XFER_DNLD_Status_E
{
    XFER_DNLD_SUCCESS                = VAL_tftpStatus_tftpSuccess, /* 1*/
    XFER_DNLD_ERROR                  ,
    XFER_DNLD_NO_IP_ADDR             ,   /* agent IP does not exist */
    XFER_DNLD_BUSY                   ,         /* download is in progress */
    XFER_DNLD_PARA_ERR               ,          /* server IP, filename is incorrect */
    XFER_DNLD_OPEN_ERROR             ,  /* open failed for TFTP.c        */
	XFER_DNLD_FILENAME_LEN_EXCEED    ,
	XFER_DNLD_UPLOAD_ERROR            ,         /* tftp_put failed for TFTP.c     */
	XFER_DNLD_DOWNLOAD_ERROR        ,          /* tftp_get failed for TFTP.c     */
	XFER_DNLD_GEN_ERROR              ,     	 /* system error               */
	XFER_DNLD_COMPLETED         
} XFER_DNLD_Status_T ;

typedef enum XFER_DNLD_TFTPStatus_E
{
    XFER_DNLD_TFTP_UNDEF_ERROR = 1,     /* = EUNDEF */
    XFER_DNLD_TFTP_FILE_NOT_FOUND,      /* = ENOTFOUND */
    XFER_DNLD_TFTP_ACCESS_VIOLATION,    /* = EACCESS */
    XFER_DNLD_TFTP_DISK_FULL,           /* = ENOSPACE */
    XFER_DNLD_TFTP_ILLEGAL_OPERATION,   /* = EBADOP */
    XFER_DNLD_TFTP_UNKNOWN_TRANSFER_ID, /* = EBADID */
    XFER_DNLD_TFTP_FILE_EXISTED,        /* = EEXISTS */
    XFER_DNLD_TFTP_NO_SUCH_USER,        /* = ENOUSER */
    XFER_DNLD_TFTP_TIMEOUT,             /* = ETIMEOUT */
    XFER_DNLD_TFTP_SEND_ERROR,          /* = ESEND */
    XFER_DNLD_TFTP_RECEIVE_ERROR,       /* = ERECEIVE */
    XFER_DNLD_TFTP_SOCKET_OPEN_ERROR,   /* = ESOCKETOPEN */
    XFER_DNLD_TFTP_SOCKET_BIND_ERROR,   /* = ESOCKETBIND */
    XFER_DNLD_TFTP_USER_CANCELED,       /* = ECANCEL */
    XFER_DNLD_TFTP_BUF_SIZE_EXCEEDS,    /* = E_BUF_SIZE_EXCEEDS */
    XFER_DNLD_TFTP_SUCCESS,
	XFER_DNLD_TFTP_COMPLETED         
} XFER_DNLD_TFTPStatus_T ;

typedef enum XFER_DNLD_Action_E
{
    XFER_DNLD_ACTION_GET_PIECE_OF_FILE,
    XFER_DNLD_ACTION_GET_WHOLE_FILE
} XFER_DNLD_Action_T;

typedef struct
{
    L_INET_AddrIp_T  server_ip;
    UI8_T   filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_FILE_NAME + 1];              /*tftp file name */
    UI8_T   *buf;                                           /*tftp use buffer*/
    UI32_T  uploadLength;                                   /*tftp download length*/
    UI32_T  tftp_retry_times;
    UI32_T  tftp_timeout;                                   /* timeout value in seconds before retry */
    XFER_DNLD_Action_T  action;                        /* XFER_DNLD_ACTION_GET_A_PIECE_OF_FILE:
                                                        * Get a piece of file from file server and
                                                        * uploadLength indicate how much data should be
                                                        * obtained.
                                                        * XFER_DNLD_ACTION_GET_A_WHOLE_FILE:
                                                        * Get a whole file from file server.
                                                        * This parameter shall be ignored on upload a file
                                                        * from switch.
                                                        */

    UI8_T   username[MAXSIZE_fileCopyServerUserName + 1];
    UI8_T   password[MAXSIZE_fileCopyServerPassword + 1];
} XFER_DNLD_Mtext_T;    

typedef enum
{
    XFER_DNLD_REMOTE_SERVER_NONE,
    XFER_DNLD_REMOTE_SERVER_TFTP,
    XFER_DNLD_REMOTE_SERVER_FTP,
    XFER_DNLD_REMOTE_SERVER_FTPS,
    XFER_DNLD_REMOTE_SERVER_SFTP,
} XFER_DNLD_RemoteServer_T;

typedef struct
{       
    BOOL_T                      is_download;	          /* is download or upload */
    XFER_DNLD_Mtext_T           *mtext;
    XFER_DNLD_RemoteServer_T    server_type;
    UI8_T                       reserved[7];
} XFER_DNLD_Msg_T;

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
void XFER_DNLD_Init(void);

/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_CreateTask                         
 *-------------------------------------------------------------
 * FUNCTION: This function create the TFTP download task.      
 *                                                             
 * INPUT   : XFER_DNLD_Msg_T *ReceiveDnldMsg                                
 *                                                             
 * OUTPUT  : none             
 * RETURN  : none   
 * Note    : none
 *--------------------------------------------------------------*/
void XFER_DNLD_CreateTask(void);

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
void XFER_DNLD_EnterMasterMode(void);

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
void XFER_DNLD_EnterSlaveMode(void);

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
void XFER_DNLD_EnterTransitionMode(void);

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
void XFER_DNLD_SetTransitionMode(void);

SYS_TYPE_Stacking_Mode_T XFER_DNLD_GetOperationMode(void);




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
BOOL_T XFER_DNLD_tftp(XFER_DNLD_Msg_T *ReceiveDnldMsg);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_SetCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when tftp error or tftp completed
 *           the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void (*fun)(UI32_T status, UI32_T download_length)
 * -------------------------------------------------------------------------*/
void XFER_DNLD_SetCallback(void (*fun)(UI32_T status, UI32_T download_length) );

/*-----------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_Abort
 *-----------------------------------------------------------------	
 * FUNCTION: CLI abort tftp task
 * INPUT   :NONE
 * OUTPUT  :NONE
 * RETURN  :NONE
 *-----------------------------------------------------------------*/
void XFER_DNLD_Abort(void); 
 
/*--------------------------------------------------------------
 * ROUTINE NAME - XFER_DNLD_GetTftpErrorMsg                         
 *-------------------------------------------------------------
 * FUNCTION: Get tftp undefined message     
 * INPUT   : None
 * OUTPUT  : tftp_error_msg
 * RETURN  : TRUE/FALSE
 *--------------------------------------------------------------*/
BOOL_T XFER_DNLD_GetTftpErrorMsg(UI8_T *tftp_error_msg);

#endif /* end _XFER_DNLD_H_ */



