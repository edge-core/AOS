#ifndef CMDFTP_H
#define CMDFTP_H

#include "sys_type.h"
#include "xfer_mgr.h"

typedef enum
{
    CMDFTP_TRANS_DIR_DOWNLOAD = 1,
    CMDFTP_TRANS_DIR_UPLOAD   = 2
} CMDFTP_TransferDirection_T;

typedef enum
{
    CMDFTP_DBG_ERROR   = 1,
    CMDFTP_DBG_WARNING = 2,
    CMDFTP_DBG_TRACE   = 4
} CMDFTP_DebugFlag_T;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_SetCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function. The registered function will
 *           be called while tftp is transmitting file.
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *
 * -------------------------------------------------------------------------*/
void CMDFTP_SetCallback(void (*fun)(UI32_T percent));

/*------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_GetErrorCode
 *-------------------------------------------------------------------------
 * FUNCTION : This function will return the error code of the current status.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : XFER_MGR_FileCopyStatus_T
 * NOTE     : None.
 *------------------------------------------------------------------------
 */
XFER_MGR_FileCopyStatus_T CMDFTP_GetErrorCode();

/*------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_SetDebugFlag
 *-------------------------------------------------------------------------
 * FUNCTION : This function will set the debug flag.
 * INPUT    : flag
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTE     : None.
 *------------------------------------------------------------------------
 */
void CMDFTP_SetDebugFlag(UI32_T flag);

/*------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_GetDebugFlag
 *-------------------------------------------------------------------------
 * FUNCTION : This function will get the debug flag.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : flag
 * NOTE     : None.
 *------------------------------------------------------------------------
 */
UI32_T CMDFTP_GetDebugFlag();

/*------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_Transfer
 *-------------------------------------------------------------------------
 * FUNCTION : This function will transfer a file between FTP server and memory
 *            stream.
 * INPUT    : ip_addr              -- FTP server IP addrss
 *            username             -- User to login
 *            password             -- Password of login user
 *            is_security          -- This parameter indicates to use FTPS to transfer or not.
 *                                    TRUE, use FTPS to transfer files
 *                                    FALSE, use FTP to transfer files
 *            dir                  -- CMDFTP_TRANS_DIR_DOWNLOAD, download from server,
 *                                    CMDFTP_TRANS_DIR_UPLOAD, download to server
 *            get_whole_file       -- This parameter be used for download file.
 *                                    TRUE, get a whole file;
 *                                    FALSE, get the specified size (buffer size)
 *                                    of the file.
 *            src_filename         -- Filename for download or upload
 *            dst_buffer           -- buffer for download/upload
 *            dst_buffer_size      -- buffer size
 *            dst_buffer_size_used -- exact amount size for donload/upload
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Support only one file transfer requirement by FTP at the same time.
 *------------------------------------------------------------------------
 */
BOOL_T CMDFTP_Transfer(L_INET_AddrIp_T *ip_addr, UI8_T *username, UI8_T *password,
    BOOL_T is_security, CMDFTP_TransferDirection_T dir, BOOL_T get_whole_file,
    UI8_T *src_filename, UI8_T *dst_buffer, UI32_T dst_buffer_size, UI32_T *dst_buffer_size_used);

#endif /* CMDFTP_H */
