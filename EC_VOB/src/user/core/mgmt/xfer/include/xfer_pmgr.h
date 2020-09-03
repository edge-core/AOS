/*-----------------------------------------------------------------------------
 * FILE NAME: XFER_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for XFER MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/25     --- Andy, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef _XFER_PMGR_MGR_H_
#define _XFER_PMGR_MGR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "xfer_dnld.h"
#include "leaf_es3626a.h"
#include "fs_type.h"
#include "xfer_type.h"
#include "sys_cpnt.h"
#include "xfer_mgr.h"
#include "l_inet.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : XFER_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for XFER_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void XFER_PMGR_InitiateProcessResource(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_CopyFile
 *------------------------------------------------------------------------
 * FUNCTION: This function will remote file copy to local
 * INPUT   : user_info_p         -- user information entry
 *           publickey_username  -- username of public key
 *           publickey_type      -- type of public key type
 *           server_p            -- remote server ip address
 *           desfile             -- destination file name
 *           srcfile             -- source file name
 *           file_type           -- file type
 *           mode                -- copy mode
 *           server_type         -- remote server type
 *           username            -- remote server user name
 *           password            -- remote server password
 *           cookie              -- the cookie of CLI working area; only CLI
 *                                  need to pass in this argument.
 *           ipc_message_q       -- the key of CSC group message queue
 *           callback            -- the callback function to notify status
 *                                  for caller; if caller cannot have a
 *                                  callback function to know the status, it
 *                                  can call XFER_MGR_GetFileCopyMgtEntry() to
 *                                  get XFER_MGR_FileCopyMgt_T, then know the
 *                                  status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE: 1.send XFER_PMGR_COPYFILE message to MgrTask (Task under mgr)
 *       2.XFER_PMGR_Mode_T:
 *         XFER_PMGR_LOCAL_TO_REMOTE --- tftp upload
 *         XFER_PMGR_REMOTE_TO_LOCAL --- tftp download
 *         XFER_PMGR_LOCAL_TO_LOCAL  ---- local copy
 *
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_CopyFile(
    XFER_MGR_UserInfo_T *user_info_p,
    char *publickey_username,
    UI32_T publickey_type,
    L_INET_AddrIp_T *server_p,
    UI8_T *destfile,
    UI8_T *srcfile,
    UI32_T file_type,
    XFER_MGR_Mode_T mode,
    XFER_MGR_RemoteServer_T server_type,
    UI8_T *username,
    UI8_T *password,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_RemoteToStream
 *------------------------------------------------------------------------
 * FUNCTION: This function will download a file to the memory stream
 * INPUT   : user_info_p       -- user information entry
 *           server_type       -- remote server type
 *           server_p          -- remote server ip address
 *           username          -- remote server user name
 *           password          -- remote server password
 *           srcfile           -- source file name
 *           file_type         -- file type
 *           x_buf             -- data buffer use to load file
 *           cookie            -- the cookie of CLI working area; only CLI
 *                                need to pass in this argument.
 *           ipc_message_q     -- the key of CSC group message queue
 *           callback          -- the callback function to notify status
 *                                for caller; if caller cannot have a
 *                                callback function to know the status, it
 *                                can call XFER_MGR_GetFileCopyMgtEntry() to
 *                                get XFER_MGR_FileCopyMgt_T, then know the
 *                                status of the last copy procedure.
 *           stream_max_length -- the maxmun length of stream
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    : send XFER_PMGR_REMOTETOSTREAM message to MgrTask (Task under mgr)
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_RemoteToStream(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    L_INET_AddrIp_T *server_p,
    UI8_T *username,
    UI8_T *password,
    UI8_T *srcfile,
    UI32_T file_type,
    UI8_T *x_buf,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status),
    UI32_T stream_max_length
);

/*------------------------------------------------------------------------
* ROUTINE NAME - XFER_PMGR_StreamToRemote_Check
*------------------------------------------------------------------------
* FUNCTION: This function will Upload the memory stream to remote file
* INPUT   : L_INET_AddrIp_T *server_p
*           UI8_T       *desfile
*           UI32_T      file_type
*           void        *cookie
*           UI32_T      ipc_message_q,
*           void        (*callback) (void *cookie, UI32_T status)
* OUTPUT  : None
* RETURN  : TRUE: succes;FALSE: if check error
* NOTE: send XFER_PMGR_STREAMTOREMOTE message to MgrTask (Task under mgr)
*------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_StreamToRemote_Check (L_INET_AddrIp_T *server_p,
                                 UI8_T  *desfile,
                                 UI32_T file_type,
                                 void *cookie,
                                 UI32_T ipc_message_q,
                                 void (*callback) (void *cookie, UI32_T status));

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_StreamToRemote_Write
 *------------------------------------------------------------------------
 * FUNCTION: This function will Upload the memory stream to remote file
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
 *           server_p       -- remote server ip address
 *           username       -- remote server user name
 *           password       -- remote server password
 *           desfile        -- destination file name
 *           file_type      -- file type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
 *           ipc_message_q  -- the key of CSC group message queue
 *           buffer         -- data buffer use to load file
 *           callback       -- the callback function to notify status
 *                             for caller; if caller cannot have a
 *                             callback function to know the status, it
 *                             can call XFER_MGR_GetFileCopyMgtEntry() to
 *                             get XFER_MGR_FileCopyMgt_T, then know the
 *                             status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    : send XFER_MGR_STREAMTOREMOTE message to MgrTask (Task under mgr)
 *------------------------------------------------------------------------
 */
BOOL_T
XFER_PMGR_StreamToRemote_Write(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    L_INET_AddrIp_T *server_p,
    UI8_T *username,
    UI8_T *password,
    UI8_T *desfile,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    UI8_T *buffer,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_StreamToRemote
 *------------------------------------------------------------------------
 * FUNCTION: This function will Upload the memory stream to remote file
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
 *           server_p       -- remote server ip address
 *           username       -- remote server user name
 *           password       -- remote server password
 *           desfile        -- destination file name
 *           file_type      -- file type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
 *           ipc_message_q  -- the key of CSC group message queue
 *           buffer         -- data buffer use to load file
 *           callback       -- the callback function to notify status
 *                             for caller; if caller cannot have a
 *                             callback function to know the status, it
 *                             can call XFER_MGR_GetFileCopyMgtEntry() to
 *                             get XFER_MGR_FileCopyMgt_T, then know the
 *                             status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE: send XFER_PMGR_STREAMTOREMOTE message to MgrTask (Task under mgr)
 *------------------------------------------------------------------------
 */
BOOL_T
XFER_PMGR_StreamToRemote(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    L_INET_AddrIp_T *server_p,
    UI8_T *username,
    UI8_T *password,
    UI8_T *desfile,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    UI8_T *buffer,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_StreamToLocal
 *------------------------------------------------------------------------
 * FUNCTION: This function will  save memory stream to local file
 * INPUT   : user_info_p    -- user information entry
 *           filename       -- local file name
 *           file_type      -- file type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
 *           ipc_message_q  -- the key of CSC group message queue
 *           callback       -- the callback function to notify status
 *                             for caller; if caller cannot have a
 *                             callback function to know the status, it
 *                             can call XFER_MGR_GetFileCopyMgtEntry() to
 *                             get XFER_MGR_FileCopyMgt_T, then know the
 *                             status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    : 1. asyn. function
 *           2. need mutual protection
 *------------------------------------------------------------------------*/
BOOL_T
XFER_PMGR_StreamToLocal(
    XFER_MGR_UserInfo_T *user_info_p,
    UI8_T *filename,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_LocalToStream
 *------------------------------------------------------------------------
 * FUNCTION: This function will from File system read file to buffer
 * INPUT   : user_info_p       -- user information entry
 *           filename          -- file name to be loaded
 *           xbuf              -- data buffer use to load file
 *           xbuf_length       -- length of xbuf
 *           stream_max_length -- the maxmun length of stream
 * OUTPUT  : xbuf_length
 * RETURN  : one of FS_RETURN_CODE_E
 * NOTE    : 1. synch. function
 *           2. pure code, so need not mutual protection
 *------------------------------------------------------------------------
 */
UI32_T
XFER_PMGR_LocalToStream(
    XFER_MGR_UserInfo_T *user_info_p,
    UI8_T *filename,
    UI8_T *xbuffer,
    UI32_T *xbuf_length,
    UI32_T stream_max_length
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_ReadSystemConfig
 *------------------------------------------------------------------------
 * FUNCTION: This function will read startup cfg file to buffer
 * INPUT   : UI8_T* xbuffer,
 *           UI32_T xbuf_length
 * OUTPUT  : xbuf_length
 * RETURN  : Actual system configuration data length(byte count)
 * NOTE    : 1. synch. function
 *           2. pure code, so need not mutual protection
 *------------------------------------------------------------------------*/
UI32_T  XFER_PMGR_ReadSystemConfig(UI8_T *xbuffer, UI32_T *xbuf_length);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_UnitToLocal
 *------------------------------------------------------------------------
 * FUNCTION: This function will send COPYUNITFILE message to MgrTask (Task under mgr)
 * INPUT   : user_info_p    -- user information entry
 *           unit_id        -- unit id
 *           desfile        -- destination file name
 *           srcfile        -- source file name
 *           file_type      -- file type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
 *           ipc_message_q  -- the key of CSC group message queue
 *           callback       -- the callback function to notify status
 *                             for caller; if caller cannot have a
 *                             callback function to know the status, it
 *                             can call XFER_MGR_GetFileCopyMgtEntry() to
 *                             get XFER_MGR_FileCopyMgt_T, then know the
 *                             status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T
XFER_PMGR_UnitToLocal(
    XFER_MGR_UserInfo_T *user_info_p,
    UI32_T unit_id,
    UI8_T *destfile,
    UI8_T *srcfile,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_LocalToUnit
 *------------------------------------------------------------------------
 * FUNCTION: This function will send COPYFILEUNIT message to MgrTask (Task under mgr)
 * INPUT   : user_info_p    -- user information entry
 *           unit_id        -- unit id
 *           destfile       -- destination file name
 *           srcfile        -- source file name
 *           file_type      -- file type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
 *           ipc_message_q  -- the key of CSC group message queue
 *           callback       -- the callback function to notify status
 *                             for caller; if caller cannot have a
 *                             callback function to know the status, it
 *                             can call XFER_MGR_GetFileCopyMgtEntry() to
 *                             get XFER_MGR_FileCopyMgt_T, then know the
 *                             status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE:
 *------------------------------------------------------------------------
 */
BOOL_T
XFER_PMGR_LocalToUnit(
    XFER_MGR_UserInfo_T *user_info_p,
    UI32_T unit_id,
    UI8_T *destfile,
    UI8_T *srcfile,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetTftpMgtEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the tftp info
 * INPUT   : tftp_mgt_entry_Info->tftp_file_type
 *           tftp_mgt_entry_Info->tftp_server
 *           tftp_mgt_entry_Info->tftp_file
 *           tftp_mgt_entry_Info->tftp_active
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_GetTftpMgtEntry(XFER_MGR_TftpMgtEntry_T *tftp_mgt_entry_Info);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetTftpFileType
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    :ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T XFER_PMGR_SetTftpFileType(UI32_T  tftp_file_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetTftpSrcFile
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T XFER_PMGR_SetTftpSrcFile(UI8_T  *tftp_src_file);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetTftpDestFile
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T XFER_PMGR_SetTftpDestFile(UI8_T  *tftp_des_file);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetTftpServer
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   : L_INET_AddrIp_T *tftp_server
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T XFER_PMGR_SetTftpServer(L_INET_AddrIp_T *tftp_server);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetTftpActive
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   : user_info_p    -- user information entry
 *           tftp_active    -- tftp server active
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T
XFER_PMGR_SetTftpActive(
    XFER_MGR_UserInfo_T *user_info_p,
    UI32_T tftp_active
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetSWVersion
 *------------------------------------------------------------------------
 * FUNCTION: This function will get local software version by file name
 * INPUT   : file_name_p - pointer of file name
 * OUTPUT  : software_version_p - pointer of software version
 * RETURN  : TRUE - success
 *           FALSE - fail
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_GetSWVersion(UI8_T *file_name_p, UI8_T *software_version_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetTftpErrorMsg
 *------------------------------------------------------------------------
 * FUNCTION: Get tftp undefined message
 * INPUT   : None
 * OUTPUT  : tftp_error_msg
 * RETURN  : TRUE/FALSE
 * NOTE    : only for CLI to get tftp undefined message when copy status is
 *           XFER_PMGR_FILE_COPY_TFTP_UNDEF_ERROR
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_GetTftpErrorMsg(UI8_T *tftp_error_msg);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetFileCopyMgtEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the file copy mgt info
 * INPUT   : None
 * OUTPUT  : file_copy_mgt
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_GetFileCopyMgtEntry(XFER_MGR_FileCopyMgt_T *file_copy_mgt);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopySrcOperType
 *------------------------------------------------------------------------
 * FUNCTION: Set the source operation type
 * INPUT   : UI32_T  src_oper_type  -- the source operation type value
 *                                     VAL_fileCopySrcOperType_file(1)
 *                                     VAL_fileCopySrcOperType_runningCfg(2)
 *                                     VAL_fileCopySrcOperType_startUpCfg(3)
 *                                     VAL_fileCopySrcOperType_tftp(4)
 *                                     VAL_fileCopySrcOperType_unit(5)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if source operation type is VAL_fileCopySrcOperType_startUpCfg,
 *           the default source file name will be boot config. But it could
 *           also be changed by XFER_PMGR_SetFileCopySrcFileName.
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopySrcOperType(UI32_T  src_oper_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopySrcFileName
 *------------------------------------------------------------------------
 * FUNCTION: Set the source file name
 * INPUT   : UI8_T  *src_file_name_p  -- the starting address of source file name
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if the source operation type is VAL_fileCopySrcOperType_runningCfg(2)
 *           or VAL_fileCopySrcOperType_startUpCfg(3), this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopySrcFileName(UI8_T  *src_file_name_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyDestOperType
 *------------------------------------------------------------------------
 * FUNCTION: Set the destination operation type
 * INPUT   : UI32_T  dest_oper_type -- the source operation type value
 *                                     VAL_fileCopyDestOperType_file(1)
 *                                     VAL_fileCopyDestOperType_runningCfg(2)
 *                                     VAL_fileCopyDestOperType_startUpCfg(3)
 *                                     VAL_fileCopyDestOperType_tftp(4)
 *                                     VAL_fileCopyDestOperType_unit(5)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if destination operation type is VAL_fileCopyDestOperType_startUpCfg,
 *           the default destination file name will be boot config. But it could
 *           also be changed by XFER_PMGR_SetFileCopyDestFileName.
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopyDestOperType(UI32_T  dest_oper_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyDestFileName
 *------------------------------------------------------------------------
 * FUNCTION: Set the destination file name
 * INPUT   : UI8_T  *dest_file_name_p   -- the starting address of destination file name
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if the destination operation type is VAL_fileCopyDestOperType_runningCfg(2)
 *           or VAL_fileCopyDestOperType_startUpCfg(3), this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopyDestFileName(UI8_T  *dest_file_name_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyFileType
 *------------------------------------------------------------------------
 * FUNCTION: Set the type of file to copy
 * INPUT   : UI32_T  file_type  -- the source operation type value
 *                                 VAL_fileCopyFileType_opcode(1)
 *                                 VAL_fileCopyFileType_config(2)
 *                                 VAL_fileCopyFileType_bootRom(3)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the source operation type or destination operation type is
 *           runningCfg(2) or startupCfg(3), this function could be ignored
 *           and the default file type would be FS_FILE_TYPE_CONFIG
 *           If the source operation type or destination operation type is
 *           unit(5), this varible could not be set to VAL_fileCopyFileType_bootRom(3).
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopyFileType(UI32_T  file_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyTftpServer
 *------------------------------------------------------------------------
 * FUNCTION: Set the IP address of the TFTP server
 * INPUT   : L_INET_AddrIp_T *tftp_server  -- the TFTP server IP address
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If neither source operation type nor destination operation type
 *           is tftp(4), this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopyTftpServer(L_INET_AddrIp_T *tftp_server);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyUnit
 *------------------------------------------------------------------------
 * FUNCTION: Set the unit id
 * INPUT   : UI32_T  unit    -- the unit id
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopyUnit(UI32_T  unit);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyAction
 *------------------------------------------------------------------------
 * FUNCTION: Start the copy operation
 * INPUT   : user_info_p    -- user information entry
 *           action         -- copy action
 *                               VAL_fileCopyAction_notCopying(1)
 *                               VAL_fileCopyAction_copy(2)
 *                               VAL_fileCopyAction_abortTftp(3)
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
 *           ipc_message_q  -- the key of CSC group message queue
 *           callback       -- the callback function to notify status
 *                             for caller; if caller cannot have a
 *                             callback function to know the status, it
 *                             can call XFER_MGR_GetFileCopyMgtEntry() to
 *                             get XFER_MGR_FileCopyMgt_T, then know the
 *                             status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
XFER_PMGR_SetFileCopyAction(
    XFER_MGR_UserInfo_T *user_info_p,
    UI32_T action,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_AbortTftp
 *------------------------------------------------------------------------
 * FUNCTION: abort what XFER is doing
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : only effect durring tftp is in progress
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_AbortTftp();

#if (TRUE == SYS_CPNT_DBSYNC_TXT)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetDualStartupCfgFileName
 *------------------------------------------------------------------------
 * FUNCTION: Get 2 fixed startup config file names in turn
 * INPUT   : None
 * OUTPUT  : file_name
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_GetDualStartupCfgFileName(UI8_T *file_name);

#endif /* #if (TRUE == SYS_CPNT_DBSYNC_TXT) */


/*------------------------------------------------------------------------
* ROUTINE NAME - XFER_PMGR_AutoDownLoad
*------------------------------------------------------------------------
* FUNCTION: This function will send COPYFILEUNIT message to MgrTask (Task under mgr)
* INPUT   : UI8_T       *unit_list,
*           UI8_T       *desfile
*           UI8_T       *srcfile,
*           UI32_T      file_type
*           BOOL_T      is_next_boot,
*           void        *cookie,
*           void        (*callback) (void *cookie, UI32_T status)
* OUTPUT  : None
* RETURN  : TRUE: succes;FALSE: if check error
* NOTE:
*------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_AutoDownLoad(UI8_T     *unit_list,
                              UI8_T     *destfile,
                              UI8_T     *srcfile,
                              UI32_T    file_type,
                              BOOL_T    is_next_boot,
                              void      *cookie,
                              UI32_T    ipc_message_q,
                              void      (*callback) (void *cookie, UI32_T status));

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetAutoDownLoad_Status
 *------------------------------------------------------------------------
 * FUNCTION:  Get auto download status
 * INPUT   : unit_id -> unit id
 * OUTPUT  : *auto_download_status
 * RETURN  : TRUE: succes;FALSE: fail
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_GetAutoDownLoad_Status(UI32_T unit_id, XFER_MGR_Auto_Download_T *download_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetNextAutoDownLoad_Status
 *------------------------------------------------------------------------
 * FUNCTION:  Get auto download status
 * INPUT   : *unit_id -> unit id
 * OUTPUT  : *auto_download_status
 * RETURN  : TRUE: succes;FALSE: fail
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_GetNextAutoDownLoad_Status(UI32_T *unit_id, XFER_MGR_Auto_Download_T *download_status);

/*------------------------------------------------------------------------
* ROUTINE NAME - XFER_PMGR_AutoConfigToUnit
*------------------------------------------------------------------------
* FUNCTION: This function will send XFER_PMGR_FUN_CONFIG_TO_UNIT message to MgrTask (Task under mgr)
* INPUT   : UI8_T       *file_name
*           void        *cookie,
*           void        (*callback) (void *cookie, UI32_T status)
* OUTPUT  : None
* RETURN  : TRUE: succes;FALSE: if check error
* NOTE:
*------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_AutoConfigToUnit(UI8_T     *file_name,
                                  void      *cookie,
                                  UI32_T    ipc_message_q,
                                  void      (*callback) (void *cookie, UI32_T status));


#if defined(JBOS)
void XFER_PMGR_SetCheckImageType(BOOL_T CheckImageType);
#endif

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the startup file base on specified
 *            type and filename, and sync master's startup file to all
 *            slave.
 * INPUT    : file_type   -- startup file type
 *                           (FS_FILE_TYPE_CONFIG / FS_FILE_TYPE_RUNTIME)
 *            file_name   -- startup file name
 *            cookie      -- CLI cookie pointer
 *            callback    -- CLI call back function pointer
 * OUTPUT   : None
 * RETURN   : TRUE        -- succes
 *            FALSE       -- failed
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_SetStartupFilename(UI32_T  file_type,
                                    UI8_T   *file_name,
                                    void    *cookie,
                                    UI32_T  ipc_message_q,
                                    void    (*callback) (void *cookie, UI32_T status));

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_WriteFile
 * ------------------------------------------------------------------------
 * FUNCTION : This function will write data from buffer to a file.
 * INPUT    : user_info_p    -- user information entry
 *            dest_file_name -- the destination file name; it should end
 *                              with '\0', the max length defined by
 *                              XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME.
 *            file_type      -- the file type; it should be one of
 *                              FS_File_Type_T.
 *            x_buf          -- the data buffer.
 *            length         -- the length of data buffer; it should less
 *                              than SYS_ADPT_MAX_FILE_SIZE.
 *            cookie         -- the cookie of CLI working area; only CLI
 *                              need to pass in this argument.
 *            callback       -- the callback function to notify status
 *                              for caller; if caller cannot have a
 *                              callback function to know the status, it
 *                              can call XFER_PMGR_GetFileCopyMgtEntry() to
 *                              get XFER_PMGR_FileCopyMgt_T, then know the
 *                              status of the last copy procedure.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : 1. if file_type is FS_FILE_TYPE_RUNTIME, will check whether
 *               file header is validate.
 *            2. This funtion will not occupy caller task, will return
 *               after error condition checking.
 * ------------------------------------------------------------------------
 */
BOOL_T
XFER_PMGR_WriteFile(
    XFER_MGR_UserInfo_T *user_info_p,
    UI8_T *dest_file_name,
    FS_File_Type_T file_type,
    UI8_T *x_buf,
    UI32_T length,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_StreamToLocal_WriteRunning
 *------------------------------------------------------------------------
 * FUNCTION: This function will save memory stream to local file
 * INPUT   : user_info_p    -- user information entry
 *           filename       -- local file name
 *           file_type      -- file type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
 *           ipc_message_q  -- the key of CSC group message queue
 *           buffer         -- data buffer use to load file
 *           callback       -- the callback function to notify status
 *                             for caller; if caller cannot have a
 *                             callback function to know the status, it
 *                             can call XFER_MGR_GetFileCopyMgtEntry() to
 *                             get XFER_MGR_FileCopyMgt_T, then know the
 *                             status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    : 1. asyn. function
 *           2. need mutual protection
 *------------------------------------------------------------------------
 */
BOOL_T
XFER_PMGR_StreamToLocal_WriteRunning(
    XFER_MGR_UserInfo_T *user_info_p,
    UI8_T *filename,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    UI8_T *buffer,
    void (*callback) (void *cookie, UI32_T status)
);

BOOL_T XFER_PMGR_StreamToLocal_check (UI8_T   *filename,
                                UI32_T  file_type,
                                void    *cookie,
                                UI32_T  ipc_message_q,
                                void    (*callback) (void *cookie, UI32_T status));

#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetInfoDownloadLength
 * ------------------------------------------------------------------------
 * FUNCTION : This function will return the length for tftp download
 * INPUT    : None
 * OUTPUT   : *out_len
 * RETURN   : None
 * NOTE     : for OAM remote FW upgrade requirment.
 * ------------------------------------------------------------------------
 */
void XFER_PMGR_GetInfoDownloadLength(UI32_T *out_len);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyOamPorts
 *------------------------------------------------------------------------
 * FUNCTION: Set the port list for OAM
 * INPUT   : *in_port_list_p - the set of ifindex in port bitmap format
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if the dest operation type != VAL_fileCopyDestOperType_oamRemote
 *           ,this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopyOamPorts(UI8_T *in_portlist_p);

#endif
#endif
#endif
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyActionFlag
 *------------------------------------------------------------------------
 * FUNCTION: set Action flag
 * INPUT   : UI32_T action  -- copy action
 *                             VAL_fileCopyAction_notCopying(1)
 *                             VAL_fileCopyAction_copy(2)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : for OAM/Cluser used.
 *           Originally, the fileCopyAction and fileCopyStatus is set by XFER
 *           Because "OAM FW upgrade" and "Cluster to member" are merged into fileMgt,
 *           add this API to set Action for OAM/Cluster
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopyActionFlag(UI32_T action);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyStatusFlag
 *------------------------------------------------------------------------
 * FUNCTION: set status flag
 * INPUT   : UI32_T status  --
 *              XFER_MGR_FILE_COPY_TFTP_UNDEF_ERROR =         XFER_DNLD_TFTP_UNDEF_ERROR,
 *              XFER_MGR_FILE_COPY_TFTP_FILE_NOT_FOUND =      XFER_DNLD_TFTP_FILE_NOT_FOUND,
 *              XFER_MGR_FILE_COPY_TFTP_ACCESS_VIOLATION =    XFER_DNLD_TFTP_ACCESS_VIOLATION,
 *              XFER_MGR_FILE_COPY_TFTP_DISK_FULL =           XFER_DNLD_TFTP_DISK_FULL,
 *              XFER_MGR_FILE_COPY_TFTP_ILLEGAL_OPERATION =   XFER_DNLD_TFTP_ILLEGAL_OPERATION,
 *              XFER_MGR_FILE_COPY_TFTP_UNKNOWN_TRANSFER_ID = XFER_DNLD_TFTP_UNKNOWN_TRANSFER_ID,
 *              XFER_MGR_FILE_COPY_TFTP_FILE_EXISTED =        XFER_DNLD_TFTP_FILE_EXISTED,
 *              XFER_MGR_FILE_COPY_TFTP_NO_SUCH_USER =        XFER_DNLD_TFTP_NO_SUCH_USER,
 *              XFER_MGR_FILE_COPY_TFTP_TIMEOUT =             XFER_DNLD_TFTP_TIMEOUT,
 *              XFER_MGR_FILE_COPY_TFTP_SEND_ERROR =          XFER_DNLD_TFTP_SEND_ERROR,
 *              XFER_MGR_FILE_COPY_TFTP_RECEIVE_ERROR =       XFER_DNLD_TFTP_RECEIVE_ERROR,
 *              XFER_MGR_FILE_COPY_TFTP_SOCKET_OPEN_ERROR =   XFER_DNLD_TFTP_SOCKET_OPEN_ERROR,
 *              XFER_MGR_FILE_COPY_TFTP_SOCKET_BIND_ERROR =   XFER_DNLD_TFTP_SOCKET_BIND_ERROR,
 *              XFER_MGR_FILE_COPY_TFTP_USER_CANCELED =       XFER_DNLD_TFTP_USER_CANCELED,
 *          	XFER_MGR_FILE_COPY_TFTP_COMPLETED,
 *          	XFER_MGR_FILE_COPY_PARA_ERROR,
 *              XFER_MGR_FILE_COPY_BUSY,
 *          	XFER_MGR_FILE_COPY_UNKNOWN,
 *              XFER_MGR_FILE_COPY_READ_FILE_ERROR,
 *              XFER_MGR_FILE_COPY_SET_STARTUP_ERROR,
 *              XFER_MGR_FILE_COPY_FILE_SIZE_EXCEED,
 *              XFER_MGR_FILE_COPY_MAGIC_WORD_ERROR,
 *              XFER_MGR_FILE_COPY_IMAGE_TYPE_ERROR,
 *              XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR,
 *              XFER_MGR_FILE_COPY_IMAGE_CHECKSUM_ERROR,
 *              XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH,
 *              XFER_MGR_FILE_COPY_WRITE_FLASH_ERR,
 *              XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING,
 *              XFER_MGR_FILE_COPY_ERROR,
 *              XFER_MGR_FILE_COPY_SUCCESS,
 *              XFER_MGR_FILE_COPY_COMPLETED,
 *              XFER_MGR_FILE_COPY_PROGRESS_UNIT1,
 *              XFER_MGR_FILE_COPY_PROGRESS_UNIT2,
 *              XFER_MGR_FILE_COPY_PROGRESS_UNIT3,
 *              XFER_MGR_FILE_COPY_PROGRESS_UNIT4,
 *              XFER_MGR_FILE_COPY_PROGRESS_UNIT5,
 *              XFER_MGR_FILE_COPY_PROGRESS_UNIT6,
 *              XFER_MGR_FILE_COPY_PROGRESS_UNIT7,
 *              XFER_MGR_FILE_COPY_PROGRESS_UNIT8,
 *              XFER_MGR_FILE_COPY_START_REBOOTING,
 *              XFER_MGR_FILE_COPY_MODULE_WRITE_FLASH_ERR
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : for OAM/Cluser used.
 *           Originally, the fileCopyAction and fileCopyStatus is set by XFER
 *           Because "OAM FW upgrade" and "Cluster to member" are merged into fileMgt,
 *           add this API to set status for OAM/Cluster
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetFileCopyStatusFlag(UI32_T status);

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetPartialProvisionStatus
 *------------------------------------------------------------------------
 * FUNCTION: set partial provision atatus
 * INPUT   : status  -- file copy status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void
XFER_PMGR_SetPartialProvisionStatus(
    UI32_T status
);
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Get TFTP retry times
 * INPUT   : None
 * OUTPUT  : UI32_T *retry_times_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_GetTftpRetryTimes(UI32_T *retry_times_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetRunningTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Get TFTP retry times
 * INPUT   : None
 * OUTPUT  : UI32_T *retry_times_p
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T XFER_PMGR_GetRunningTftpRetryTimes(UI32_T *retry_times_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Set TFTP retry times
 * INPUT   : UI32_T retry_times
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Range -- XFER_MGR_MIN_TFTP_RETRY_TIMES~XFER_MGR_MAX_TFTP_RETRY_TIMES
 *------------------------------------------------------------------------*/
BOOL_T XFER_PMGR_SetTftpRetryTimes(UI32_T retry_times);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetTftpTimeout
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves TFTP timeout value in seconds before retry
 * INPUT   : None
 * OUTPUT  : timeout_p  - timeout value in seconds
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_GetTftpTimeout(UI32_T *timeout_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetRunningTftpTimeout
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves TFTP timeout value in seconds before retry
 * INPUT   : None
 * OUTPUT  : timeout_p  - timeout value in seconds
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE    : None
 *------------------------------------------------------------------------
 */
UI32_T XFER_PMGR_GetRunningTftpTimeout(UI32_T *timeout_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Set TFTP timeout value in seconds before retry
 * INPUT   : timeout    - timeout value in seconds
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_SetTftpTimeout(UI32_T timeout);

BOOL_T XFER_PMGR_SetFileCopyPublickeyType(UI32_T  key_type);


BOOL_T XFER_PMGR_SetFileCopyUserName(char *username);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyServerInetAddress
 *------------------------------------------------------------------------
 * FUNCTION: Set the inet address of the server.
 * INPUT   : server
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_SetFileCopyServerInetAddress(L_INET_AddrIp_T *server_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyServerUserName
 *------------------------------------------------------------------------
 * FUNCTION: Set the user name.
 * INPUT   : username
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_SetFileCopyServerUserName(UI8_T *username);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetFileCopyServerPassword
 *------------------------------------------------------------------------
 * FUNCTION: Set the password.
 * INPUT   : password
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_SetFileCopyServerPassword(UI8_T *password);

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetAutoOpCodeUpgradeStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set auto image upgrade status
 * INPUT   : status - auto image upgrade status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_SetAutoOpCodeUpgradeStatus(UI32_T status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetAutoOpCodeUpgradeStatus
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade status
 * INPUT   : None
 * OUTPUT  : status_p - auto image upgrade status
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_GetAutoOpCodeUpgradeStatus(UI32_T *status_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetRunningAutoOpCodeUpgradeStatus
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade status
 * INPUT   : None
* OUTPUT  : status_p - auto image upgrade status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE    : None
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T XFER_PMGR_GetRunningAutoOpCodeUpgradeStatus(UI32_T *status_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetAutoOpCodeUpgradeReloadStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set auto image upgrade reload status
 * INPUT   : status - auto image upgrade reload status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_SetAutoOpCodeUpgradeReloadStatus(UI32_T status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetAutoOpCodeUpgradeReloadStatus
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade reload status
 * INPUT   : None
 * OUTPUT  : status_p - auto image upgrade reload status
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_GetAutoOpCodeUpgradeReloadStatus(UI32_T *status_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetRunningAutoOpCodeUpgradeReloadStatus
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade reload status
 * INPUT   : None
* OUTPUT  : status_p - auto image upgrade reload status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE    : None
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T XFER_PMGR_GetRunningAutoOpCodeUpgradeReloadStatus(UI32_T *status_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_SetAutoOpCodeUpgradePath
 *------------------------------------------------------------------------
 * FUNCTION: Set auto image upgrade search path
 * INPUT   : path_p - search path
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : The last character of the path is always a "/".
 *           The scheme and host of the path are required.
 *           The path should not include the searched file name.
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_SetAutoOpCodeUpgradePath(const char *path_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetAutoOpCodeUpgradePath
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade search path
 * INPUT   : None
 * OUTPUT  : path_p - search path
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_GetAutoOpCodeUpgradePath(char *path_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetRunningAutoOpCodeUpgradePath
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade search path
 * INPUT   : None
 * OUTPUT  : path_p - search path
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE    : None
 *------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T XFER_PMGR_GetRunningAutoOpCodeUpgradePath(char *path_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_PMGR_GetAutoOpCodeUpgradeFileName
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade search file name
 * INPUT   : None
 * OUTPUT  : filename_p - search image's file name
 * RETURN  : TRUE/FALSE
 * NOTE    : The file name is read-only
 *------------------------------------------------------------------------
 */
BOOL_T XFER_PMGR_GetAutoOpCodeUpgradeFileName(char *filename_p);
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */

#endif /* #ifndef XFR_PMGR_H */
