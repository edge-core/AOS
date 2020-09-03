/* Project Name: Mercury
 * Module Name : XFER_MGR.h
 * Abstract    :
 * Purpose     :
 *
 * History :
 *          Date        Modifier        Reason
 *          2001/10/11  BECK CHEN       Create this file
 *          2002/12/05  Erica Li        Add XFER_MGR_FileCopyStatus_T
 *                                      Add XFER_MGR_FileCopyMgt_T
 *                                      Add XFER_MGR_GetFileCopyMgtEntry()
 *                                      Add XFER_MGR_SetFileCopySrcOperType()
 *                                      Add XFER_MGR_SetFileCopySrcFileName()
 *                                      Add XFER_MGR_SetFileCopyDestOperType()
 *                                      Add XFER_MGR_SetFileCopyDestFileName()
 *                                      Add XFER_MGR_SetFileCopyFileType()
 *                                      Add XFER_MGR_SetFileCopyTftpServer()
 *                                      Add XFER_MGR_SetFileCopyUnit()
 *                                      Add XFER_MGR_SetFileCopyAction()
 *          2002/12/09  Erica Li        Change spec of XFER_MGR_StreamToRemote() to make XFER alloc/free
 *                                      the copy buffer itself.
 *                                      Change spec of XFER_MGR_StreamToLocal() to make XFERK alloc/free
 *                                      the copy buffer itself.
 *          2003/01/29  Erica Li        Add XFER_MGR_CopyTftpToStartupOpCode()
 *                                      Add XFER_MGR_SetTftpToStartupOpCodeAction()
 *          2003/07/28  Erica Li        Add XFER_MGR_GetLastSWUpgradeIp()
 *                                      Add XFER_MGR_GetLastSWUpgradeFileName()
 *                                      Add XFER_MGR_GetLastSavingConfigIp()
 *                                      Add XFER_MGR_GetLastSavingConfigFileName()
 *                                      Add XFER_MGR_GetLastRestoringConfigIp()
 *                                      Add XFER_MGR_GetLastRestoringConfigFileName()
 *          2003/07/31  Erica Li        Add XFER_MGR_AbortTftp()
 *                                      Add XFER_MGR_GetUsingStatus()
 *          2003/08/06  Erica Li        Add XFER_MGR_GetDualStartupCfgFileName()
 *          2003/08/14  Erica Li        Add XFER_MGR_GetTftpRetryTimes()
 *                                      Add XFER_MGR_GetRunningTftpRetryTimes()
 *                                      Add XFER_MGR_SetTftpRetryTimes()
 *          2003/10/23  Pttch           Add Config file sync and image auto download
 *          2003/12/27  Erica Li        Add hot swap functions
 *
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 *
 * Note    :
 */

#ifndef _XFER_MGR_MGR_H_
#define _XFER_MGR_MGR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "xfer_dnld.h"
#include "leaf_es3626a.h"
#include "fs_type.h"
#include "xfer_type.h"
#include "sys_cpnt.h"
#include "l_inet.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define XFER_MGR_IPCMSG_TYPE_SIZE sizeof(union XFER_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    XFER_MGR_IPC_COPYFILE,
    XFER_MGR_IPC_REMOTETOSTREAM,
    XFER_MGR_IPC_STREAMTOREMOTE,
    XFER_MGR_IPC_STREAMTOREMOTE_CHECK,
    XFER_MGR_IPC_STREAMTOREMOTE_WRITING,
    XFER_MGR_IPC_STREAMTOLOCAL,
    XFER_MGR_IPC_LOCALTOSTREAM,
    XFER_MGR_IPC_READSYSTEMCONFIG,
    XFER_MGR_IPC_UNITTOLOCAL,
    XFER_MGR_IPC_LOCALTOUNIT,
    XFER_MGR_IPC_GETTFTPMGTENTRY,
    XFER_MGR_IPC_SETTFTPFILETYPE,
    XFER_MGR_IPC_SETTFTPSRCFILE,
    XFER_MGR_IPC_SETTFTPDESTFILE,
    XFER_MGR_IPC_SETTFTPSERVER,
    XFER_MGR_IPC_SETTFTPACTIVE,
    XFER_MGR_IPC_GETSWVERSION,
    XFER_MGR_IPC_GETTFTPERRORMSG,
    XFER_MGR_IPC_GETFILECOPYMGTENTRY,
    XFER_MGR_IPC_SETFILECOPYSRCOPERTYPE,
    XFER_MGR_IPC_SETFILECOPYSRCFILENAME,
    XFER_MGR_IPC_SETFILECOPYDESTOPERTYPE,
    XFER_MGR_IPC_SETFILECOPYDESTFILENAME,
    XFER_MGR_IPC_SETFILECOPYFILETYPE,
    XFER_MGR_IPC_SETFILECOPYTFTPSERVER,
    XFER_MGR_IPC_SETFILECOPYUNIT,
    XFER_MGR_IPC_SETFILECOPYACTION,
    XFER_MGR_IPC_SETPUBLICKEYTYPE,
    XFER_MGR_IPC_SETCOPYUSERNAME,
    XFER_MGR_IPC_ABORTTFTP,
    XFER_MGR_IPC_GETDUALSTARTUPCFGFILENAME,
    XFER_MGR_IPC_AUTODOWNLOAD,
    XFER_MGR_IPC_GETAUTODOWNLOAD_STATUS,
    XFER_MGR_IPC_GETNEXTAUTODOWNLOAD_STATUS,
    XFER_MGR_IPC_AUTOCONFIGTOUNIT,
    XFER_MGR_IPC_SETCHECKIMAGETYPE,
    XFER_MGR_IPC_SETSTARTUPFILENAME,
    XFER_MGR_IPC_WRITEFILE,
    XFER_MGR_IPC_STREAMTOLOCAL_4RUNNING,
#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
    XFER_MGR_IPC_GETDOWNLOAD_LENGTH,
    XFER_MGR_IPC_SETOAMPORTLIST,
#endif
#endif
#endif
#endif
    XFER_MGR_IPC_FILECOPY_ACTION_FLAG,
    XFER_MGR_IPC_FILECOPY_STATUS_FLAG,
    XFER_MGR_IPC_SET_PARTIAL_PROVISION_STATUS,
    XFER_MGR_IPC_STREAMTOLOCAL_4WRITING,
    XFER_MGR_IPC_SET_TFTP_RETRY_TIMES,
    XFER_MGR_IPC_GET_TFTP_RETRY_TIMES,
    XFER_MGR_IPC_GET_RUNNING_TFTP_RETRY_TIMES,
    XFER_MGR_IPC_SET_TFTP_TIMEOUT,
    XFER_MGR_IPC_GET_TFTP_TIMEOUT,
    XFER_MGR_IPC_GET_RUNNING_TFTP_TIMEOUT,
    XFER_MGR_IPC_SETFILECOPYSERVERINETADDRESS,
    XFER_MGR_IPC_SETFILECOPYSERVERUSERNAME,
    XFER_MGR_IPC_SETFILECOPYSERVERPASSWORD,
    XFER_MGR_IPC_SETAUTOOPCODEUPGRADESTATUS,
    XFER_MGR_IPC_GETAUTOOPCODEUPGRADESTATUS,
    XFER_MGR_IPC_GETRUNNINGAUTOOPCODEUPGRADESTATUS,
    XFER_MGR_IPC_SETAUTOOPCODEUPGRADERELOADSTATUS,
    XFER_MGR_IPC_GETAUTOOPCODEUPGRADERELOADSTATUS,
    XFER_MGR_IPC_GETRUNNINGAUTOOPCODEUPGRADERELOADSTATUS,
    XFER_MGR_IPC_SETAUTOOPCODEUPGRADEPATH,
    XFER_MGR_IPC_GETAUTOOPCODEUPGRADEPATH,
    XFER_MGR_IPC_GETRUNNINGAUTOOPCODEUPGRADEPATH,
    XFER_MGR_IPC_GETAUTOOPCODEUPGRADEFILENAME,
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in LACP_MGR_IpcMsg_T.data
 */
#define XFER_MGR_GET_MSG_SIZE(field_name)                       \
            (XFER_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((XFER_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
#define XFER_MGR_Create_Tasks_ErrNo 0

#define XFER_MGR_DFLT_USERNAME      "Anonymous"
#define XFER_MGR_DFLT_PASSWORD      ""

enum XFER_MGR_FUN_E
{
    XFER_MGR_FUN_CREATE_TASK = 0,
    XFER_MGR_FUN_COPY_FILE,
    XFER_MGR_FUN_REMOTE_TO_STREAM,
    XFER_MGR_FUN_STREAM_TO_REMOTE,
    XFER_MGR_FUN_STREAM_TO_LOCAL,
    XFER_MGR_FUN_LOCAL_TO_STREAM,
    XFER_MGR_FUN_SET_TFTP_SRC_FILE,
    XFER_MGR_FUN_SET_TFTP_DEST_FILE,
    XFER_MGR_FUN_SET_TFTP_SERVER,
    XFER_MGR_FUN_SET_TFTP_ACTIVE,
    XFER_MGR_FUN_UNIT_TO_LOCAL,
    XFER_MGR_FUN_LOCAL_TO_UNIT,   /*for auto download*/
    XFER_MGR_FUN_CONFIG_TO_UNIT   /*temp for xgs stacking*/
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#define XFER_MGR_MODE_TABLE                                               \
    MODE(XFER_MGR_LOCAL_TO_REMOTE,     XFER_MGR_REMOTE_TRANSFER_UPLOAD)   \
    MODE(XFER_MGR_REMOTE_TO_LOCAL,     XFER_MGR_REMOTE_TRANSFER_DOWNLOAD) \
    MODE(XFER_MGR_LOCAL_TO_LOCAL,      XFER_MGR_REMOTE_TRANSFER_NA)       \
    MODE(XFER_MGR_USBDISK_TO_LOCAL,    XFER_MGR_REMOTE_TRANSFER_DOWNLOAD) \
    MODE(XFER_MGR_LOCAL_TO_STARTUP,    XFER_MGR_REMOTE_TRANSFER_NA)       \
    MODE(XFER_MGR_STARTUP_TO_LOCAL,    XFER_MGR_REMOTE_TRANSFER_NA)       \
    MODE(XFER_MGR_REMOTE_TO_STARTUP,   XFER_MGR_REMOTE_TRANSFER_DOWNLOAD) \
    MODE(XFER_MGR_STARTUP_TO_REMOTE,   XFER_MGR_REMOTE_TRANSFER_UPLOAD)   \
    MODE(XFER_MGR_REMOTE_TO_STREAM,    XFER_MGR_REMOTE_TRANSFER_DOWNLOAD) \
    MODE(XFER_MGR_REMOTE_TO_PUBLICKEY, XFER_MGR_REMOTE_TRANSFER_DOWNLOAD) \
    MODE(XFER_MGR_STARTUP_TO_STREAM,   XFER_MGR_REMOTE_TRANSFER_NA)       \
    MODE(XFER_MGR_STREAM_TO_STARTUP,   XFER_MGR_REMOTE_TRANSFER_NA)

typedef enum
{
    XFER_MGR_REMOTE_TRANSFER_DOWNLOAD,
    XFER_MGR_REMOTE_TRANSFER_UPLOAD,
    XFER_MGR_REMOTE_TRANSFER_NA
} XFER_MGR_REMOTE_TRANSFER_T;

typedef enum
{
#define MODE(mode, remoteTransfer) mode,
    XFER_MGR_MODE_TABLE
#undef MODE
} XFER_MGR_Mode_T;

typedef enum
{
    XFER_MGR_TFTPSTATUSUNKNOWN =        0,
    XFER_MGR_SUCCESS =                  XFER_DNLD_SUCCESS,
    XFER_MGR_ERROR =                    XFER_DNLD_ERROR,
    XFER_MGR_NO_IP_ADDR =               XFER_DNLD_NO_IP_ADDR,                   /* agent IP does not exist */
    XFER_MGR_BUSY =                     XFER_DNLD_BUSY,                         /* download is in progress */
    XFER_MGR_PARA_ERR =                 XFER_DNLD_PARA_ERR,                     /* server IP, filename is incorrect */
    XFER_MGR_OPEN_ERROR =               XFER_DNLD_OPEN_ERROR,                   /* open failed for TFTP.c        */
    XFER_MGR_FILENAME_LEN_EXCEED =      XFER_DNLD_FILENAME_LEN_EXCEED,
    XFER_MGR_UPLOAD_ERROR =             XFER_DNLD_UPLOAD_ERROR  ,               /* tftp_put failed for TFTP.c     */
    XFER_MGR_DOWNLOAD_ERROR =           XFER_DNLD_DOWNLOAD_ERROR,               /* tftp_get failed for TFTP.c     */
    XFER_MGR_GEN_ERROR =                XFER_DNLD_GEN_ERROR,                    /* system error               */
    XFER_MGR_TFTP_COMPLETED =           XFER_DNLD_COMPLETED,
    XFER_MGR_WRITE_FLASH_FINISH ,                                                /* flash write finish         */
    XFER_MGR_WRITE_FLASH_ERR ,                                                   /* flash write error          */
    XFER_MGR_WRITE_FLASH_PROGRAMMING ,
    XFER_MGR_READ_FILE_ERR,
    XFER_MGR_HEADER_CHECKSUM_ERROR,
    XFER_MGR_IMAGE_CHECKSUM_ERROR,
    XFER_MGR_IMAGE_TYPE_ERROR,
    XFER_MGR_COMPLETED
} XFER_MGR_Status_T;

typedef enum
{
    XFER_MGR_FILE_COPY_TFTP_ILLEGAL_OPERATION =     XFER_DNLD_TFTP_ILLEGAL_OPERATION,
    XFER_MGR_FILE_COPY_TFTP_UNKNOWN_TRANSFER_ID =   XFER_DNLD_TFTP_UNKNOWN_TRANSFER_ID,
    XFER_MGR_FILE_COPY_TFTP_FILE_EXISTED =          XFER_DNLD_TFTP_FILE_EXISTED,
    XFER_MGR_FILE_COPY_TFTP_NO_SUCH_USER =          XFER_DNLD_TFTP_NO_SUCH_USER,
    XFER_MGR_FILE_COPY_TFTP_SEND_ERROR =            XFER_DNLD_TFTP_SEND_ERROR,
    XFER_MGR_FILE_COPY_TFTP_RECEIVE_ERROR =         XFER_DNLD_TFTP_RECEIVE_ERROR,
    XFER_MGR_FILE_COPY_TFTP_SOCKET_OPEN_ERROR =     XFER_DNLD_TFTP_SOCKET_OPEN_ERROR,
    XFER_MGR_FILE_COPY_TFTP_SOCKET_BIND_ERROR =     XFER_DNLD_TFTP_SOCKET_BIND_ERROR,
    XFER_MGR_FILE_COPY_TFTP_USER_CANCELED =         XFER_DNLD_TFTP_USER_CANCELED,
    XFER_MGR_FILE_COPY_PARA_ERROR =                 VAL_fileCopyStatus_fileCopyParaError, /* server IP, filename is incorrect */
    XFER_MGR_FILE_COPY_BUSY =                       VAL_fileCopyStatus_fileCopyBusy, /* download is in progress */
    XFER_MGR_FILE_COPY_UNKNOWN =                    VAL_fileCopyStatus_fileCopyUnknown,
    XFER_MGR_FILE_COPY_READ_FILE_ERROR =            VAL_fileCopyStatus_fileCopyReadFileError,
    XFER_MGR_FILE_COPY_SET_STARTUP_ERROR =          VAL_fileCopyStatus_fileCopySetStartupError,
    XFER_MGR_FILE_COPY_FILE_SIZE_EXCEED =           VAL_fileCopyStatus_fileCopyFileSizeExceed,
    XFER_MGR_FILE_COPY_MAGIC_WORD_ERROR =           VAL_fileCopyStatus_fileCopyMagicWordError,
    XFER_MGR_FILE_COPY_IMAGE_TYPE_ERROR =           VAL_fileCopyStatus_fileCopyImageTypeError,
    XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR =      VAL_fileCopyStatus_fileCopyHeaderChecksumError,
    XFER_MGR_FILE_COPY_IMAGE_CHECKSUM_ERROR =       VAL_fileCopyStatus_fileCopyImageChecksumError,
    XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH =         VAL_fileCopyStatus_fileCopyWriteFlashFinish, /* flash write finish */
    XFER_MGR_FILE_COPY_WRITE_FLASH_ERR =            VAL_fileCopyStatus_fileCopyWriteFlashError, /* flash write error */
    XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING =    VAL_fileCopyStatus_fileCopyWriteFlashProgramming,
    XFER_MGR_FILE_COPY_ERROR =                      VAL_fileCopyStatus_fileCopyError,
    XFER_MGR_FILE_COPY_SUCCESS =                    VAL_fileCopyStatus_fileCopySuccess,
    XFER_MGR_FILE_COPY_COMPLETED =                  VAL_fileCopyStatus_fileCopyCompleted,
    XFER_MGR_FILE_COPY_FILE_NOT_FOUND =             VAL_fileCopyStatus_fileCopyFileNotFound,
    XFER_MGR_FILE_COPY_SERVER_PERMISSION_DENIED =   VAL_fileCopyStatus_fileCopyServerPermissionDenied,
    XFER_MGR_FILE_COPY_STORAGE_FULL =               VAL_fileCopyStatus_fileCopyStorageFull,
    XFER_MGR_FILE_COPY_CONNECT_ERROR =              VAL_fileCopyStatus_fileCopyConnectError,
    XFER_MGR_FILE_COPY_SERVER_NOT_IN_SERVICE =      VAL_fileCopyStatus_fileCopyServerNotInService,
    XFER_MGR_FILE_COPY_DATA_CONNECTION_OPEN_ERROR = VAL_fileCopyStatus_fileCopyDataConnectionOpenError,
    XFER_MGR_FILE_COPY_LOG_IN_ERROR =               VAL_fileCopyStatus_fileCopyLogInError,
    XFER_MGR_FILE_COPY_INVALID_FILE_NAME =          VAL_fileCopyStatus_fileCopyInvalidFileName,
    XFER_MGR_FILE_COPY_SERVER_NOT_ACCEPT_PROVIDED_CIPHERS = VAL_fileCopyStatus_fileCopyServerNotAcceptProvidedCiphers,
    XFER_MGR_FILE_COPY_SERVER_NOT_SUPPORT_FTPS =    VAL_fileCopyStatus_fileCopyServerNotSupportFtps,
    XFER_MGR_FILE_COPY_FILE_UNAVAILABLE =           VAL_fileCopyStatus_fileCopyFileUnavailable,
    XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR =         VAL_fileCopyStatus_fileCopyUnclassifiedError,
    XFER_MGR_FILE_COPY_TIMEOUT =                    VAL_fileCopyStatus_fileCopyTimeout,
    XFER_MGR_FILE_COPY_PROJECT_ID_ERROR =           VAL_fileCopyStatus_fileCopyProjectIdError,
    XFER_MGR_FILE_COPY_FILE_NUM_EXCEED =            VAL_fileCopyStatus_fileCopyFileNumExceed,           /* EXCEED_MAX_NUM_OF_FILES */
    XFER_MGR_FILE_COPY_SAME_VERSION =               VAL_fileCopyStatus_fileCopySameVersion,
    XFER_MGR_FILE_COPY_OP_CODE_VERSION_NOT_SUPPORTED_BY_LICENSE = VAL_fileCopyOpCodeVersionNotSupportedByLicense,

    XFER_MGR_FILE_COPY_PROGRESS_UNIT1,
    XFER_MGR_FILE_COPY_PROGRESS_UNIT2,
    XFER_MGR_FILE_COPY_PROGRESS_UNIT3,
    XFER_MGR_FILE_COPY_PROGRESS_UNIT4,
    XFER_MGR_FILE_COPY_PROGRESS_UNIT5,
    XFER_MGR_FILE_COPY_PROGRESS_UNIT6,
    XFER_MGR_FILE_COPY_PROGRESS_UNIT7,
    XFER_MGR_FILE_COPY_PROGRESS_UNIT8,
    XFER_MGR_FILE_COPY_WRITE_LOADER_TO_FLASH,
} XFER_MGR_FileCopyStatus_T;

typedef enum
{
    XFER_MGR_SW_UPGRADING = 1,
    XFER_MGR_CFG_SAVING,
    XFER_MGR_CFG_RESTORING,
    XFER_MGR_IN_OTHER_SERVICE,
    XFER_MGR_NOT_IN_SERVICE
} XFER_MGR_UsingStatus_T;

typedef struct
{
   UI32_T                   tftpFileType;
   UI8_T                    tftpSrcFile[MAXSIZE_tftpSrcFile + 1];
   UI8_T                    tftpDestFile[MAXSIZE_tftpDestFile + 1];
   L_INET_AddrIp_T          tftpServer;
   UI32_T                   tftpAction;
   XFER_MGR_Status_T        tftpStatus;
}XFER_MGR_TftpMgtEntry_T;

typedef struct
{
    UI32_T                     session_type;
    char                       user_name[SYS_ADPT_MAX_USER_NAME_LEN+1];
    L_INET_AddrIp_T            user_ip;
    UI8_T                      user_mac[SYS_ADPT_MAC_ADDR_LEN];
} XFER_MGR_UserInfo_T;

typedef struct
{
    UI8_T                       src_oper_type;
    UI8_T                       src_file_name[MAXSIZE_fileCopySrcFileName + 1];
    UI8_T                       dest_oper_type;
    UI8_T                       dest_file_name[MAXSIZE_fileCopyDestFileName + 1];
    FS_File_Type_T              file_type;

    UI8_T                       unit;
    UI8_T                       action;
    XFER_MGR_FileCopyStatus_T   status; /* The status of the last copy procedure. */

    /* Only when the status is XFER_MGR_FILE_COPY_TFTP_UNDEF_ERROR this value is meaningful
     */
    UI8_T                       tftp_error_msg[MAXSIZE_fileCopyTftpErrMsg + 1];

#if (SYS_CPNT_SFTP == TRUE)
    UI32_T                      host_key_status;
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

    L_INET_AddrIp_T             server_address;

    UI32_T                      publickey;/*publickey type */
    char                        publickey_username[SYS_ADPT_MAX_USER_NAME_LEN + 1];   /*username to download public key from TFTP server*/

    UI8_T                       username[MAXSIZE_fileCopyServerUserName + 1];
    UI8_T                       password[MAXSIZE_fileCopyServerPassword + 1];

#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
    UI8_T                       oam_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]; /* for OAM use */
#endif
#endif
#endif
#endif

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
    UI32_T                      ipc_message_q;
    void                        *cookie;
    void                        (*callback) (void *cookie, UI32_T status);
    UI8_T                       *buf;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

} XFER_MGR_FileCopyMgt_T;

typedef struct
{
    XFER_MGR_FileCopyStatus_T  auto_download_status;
    UI32_T                     copy_action;
} XFER_MGR_Auto_Download_T;

typedef enum
{
    XFER_MGR_NONE_FUNC_NO                       = 0,
    XFER_MGR_COPYFILE_FUNC_NO                   = 1,
    XFER_MGR_REMOTETOSTREAM_FUNC_NO             = 2,
    XFER_MGR_STREAMTOREMOTE_FUNC_NO             = 3,
    XFER_MGR_STREAMTOLOCAL_FUNC_NO              = 4,
    XFER_MGR_LOCALTOSTREAM_FUNC_NO              = 5,
    XFER_MGR_UNITTOLOCAL_FUNC_NO                = 6,
    XFER_MGR_LOCALTOUNIT_FUNC_NO                = 7,
    XFER_MGR_SETTFTPSRCFILE_FUNC_NO             = 8,
    XFER_MGR_SETTFTPDESTFILE_FUNC_NO            = 9,
    XFER_MGR_SETTFTPSERVER_FUNC_NO              = 10,
    XFER_MGR_SETTFTPACTIVE_FUNC_NO              = 11,
    XFER_MGR_GETSWVERSION_FUNC_NO               = 12,
    XFER_MGR_SETFILECOPYSRCOPERTYPE_FUNC_NO     = 13,
    XFER_MGR_SETFILECOPYSRCFILENAME_FUNC_NO     = 14,
    XFER_MGR_SETFILECOPYDESTOPERTYPE_FUNC_NO    = 15,
    XFER_MGR_SETFILECOPYDESTFILENAME_FUNC_NO    = 16,
    XFER_MGR_SETFILECOPYFILETYPE_FUNC_NO        = 17,
    XFER_MGR_SETFILECOPYTFTPSERVER_FUNC_NO      = 18,
    XFER_MGR_SETFILECOPYUNIT_FUNC_NO            = 19,
    XFER_MGR_SETFILECOPYACTION_FUNC_NO          = 20,
    XFER_MGR_COPYFILEFILE_FUNC_NO               = 21,
    XFER_MGR_COPYFILERUNNING_FUNC_NO            = 22,
    XFER_MGR_COPYFILETFTP_FUNC_NO               = 23,
    XFER_MGR_COPYRUNNINGFILE_FUNC_NO            = 24,
    XFER_MGR_COPYTFTPFILE_FUNC_NO               = 25,
    XFER_MGR_COPYTTFPRUNNING_FUNC_NO            = 26,
    XFER_MGR_COPYUNITFILE_FUNC_NO               = 27,
    XFER_MGR_COPYFILEUNIT_FUNC_NO               = 28,
    XFER_MGR_SETFILECOPYSERVERINETADDRESS_FUNC_NO       = 29,
    XFER_MGR_AUTOUPGRADE_FUNC_NO                = 30,
} XFER_MGR_UI_MESSAGE_FUNC_NO_T;

typedef enum
{
    XFER_MGR_REMOTE_SERVER_NONE = XFER_DNLD_REMOTE_SERVER_NONE,
    XFER_MGR_REMOTE_SERVER_TFTP = XFER_DNLD_REMOTE_SERVER_TFTP,
    XFER_MGR_REMOTE_SERVER_FTP  = XFER_DNLD_REMOTE_SERVER_FTP,
    XFER_MGR_REMOTE_SERVER_FTPS = XFER_DNLD_REMOTE_SERVER_FTPS,
    XFER_MGR_REMOTE_SERVER_SFTP = XFER_DNLD_REMOTE_SERVER_SFTP,
} XFER_MGR_RemoteServer_T;

/* IPC message structure
 */
typedef struct
{
    union XFER_MGR_IpcMsg_Type_U
    {
        UI32_T                     cmd;
        BOOL_T                     ret_bool;
        UI32_T                     ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        struct
        {
            XFER_MGR_UserInfo_T  arg_user_info;
        } arg_grp_bool_setuserinfo;

        struct
        {
            XFER_MGR_UserInfo_T      arg_user_info;
            L_INET_AddrIp_T          arg_server_ip;
            UI8_T                    arg_destfile[MAXSIZE_fileCopyDestFileName+ 1];
            UI8_T                    arg_srcfile[MAXSIZE_fileCopySrcFileName+ 1];
            UI32_T                   arg_file_type;
            XFER_MGR_Mode_T          arg_mode;
            XFER_MGR_RemoteServer_T  arg_server_type;
            UI8_T                    arg_username[MAXSIZE_fileCopyServerUserName + 1];
            UI8_T                    arg_password[MAXSIZE_fileCopyServerPassword + 1];
            void                     *arg_cookie;
            UI32_T                   arg_ipc_message_q;
            void                     (*arg_callback) (void *cookie, UI32_T status);
            UI32_T                   arg_publickey_type;
            char                     arg_publickey_username[SYS_ADPT_MAX_USER_NAME_LEN + 1];
        } arg_grp_bool_copyfile;

        struct
        {
            XFER_MGR_UserInfo_T      arg_user_info;
            XFER_MGR_RemoteServer_T  arg_server_type;
            L_INET_AddrIp_T          arg_server_ip;
            UI8_T                    arg_srcfile[XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME+ 1];
            UI8_T                    arg_username[MAXSIZE_fileCopyServerUserName + 1];
            UI8_T                    arg_password[MAXSIZE_fileCopyServerPassword + 1];
            UI32_T                   arg_file_type;
            I32_T                    arg_x_buf_offset;
            void                     *arg_cookie;
            UI32_T                   arg_ipc_message_q;
            void                     (*arg_callback) (void *cookie, UI32_T status);
            UI32_T                   arg_stream_max_length;
        } arg_grp_bool_remotetoStream;

        struct
        {
            XFER_MGR_UserInfo_T      arg_user_info;
            XFER_MGR_RemoteServer_T  arg_server_type;
            L_INET_AddrIp_T          arg_server_ip;
            UI8_T                    arg_username[MAXSIZE_fileCopyServerUserName + 1];
            UI8_T                    arg_password[MAXSIZE_fileCopyServerPassword + 1];
            UI8_T                    arg_desfile[XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME+ 1];
            UI32_T                   arg_file_type;
            void                     *arg_cookie;
            UI32_T                   arg_ipc_message_q;
            UI32_T                   arg_buffer_length;
            I32_T                    arg_buffer_offset;
            void                     (*arg_callback) (void *cookie, UI32_T status);
        } arg_grp_bool_streamtoremote;

        struct
        {
            XFER_MGR_UserInfo_T  arg_user_info;
            UI8_T                arg_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME+ 1];
            UI32_T               arg_file_type;
            void                 *arg_cookie;
            UI32_T               arg_ipc_message_q;
            UI32_T               arg_buffer_length;
            I32_T                arg_buffer_offset;
            void                 (*arg_callback) (void *cookie, UI32_T status);
        } arg_grp_bool_streamtolocal;

        struct
        {
            XFER_MGR_UserInfo_T  arg_user_info;
            UI8_T                arg_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME+ 1];
            UI8_T                *temp_buffer;
            I32_T                arg_xbuffer_offset;
            UI32_T               arg_xbuf_length;
            UI32_T               arg_stream_max_length;
        } arg_grp_ui32_localtostream;

        struct
	    {
            I32_T       arg_xbuffer_offset;
            UI32_T      arg_xbuf_length;
	    } arg_grp_ui32_readsystemconfig;

        struct
        {
            XFER_MGR_UserInfo_T  arg_user_info;
            UI32_T               arg_unit_id;
            UI8_T                arg_destfile[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME+ 1];
            UI8_T                arg_srcfile[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME+ 1];
            UI32_T               arg_file_type;
            void                 *arg_cookie;
            UI32_T               arg_ipc_message_q;
            void                 (*arg_callback) (void *cookie, UI32_T status);
        } arg_grp_bool_unittolocal;

        struct
        {
            XFER_MGR_UserInfo_T  arg_user_info;
            UI32_T      arg_unit_id;
            UI8_T       arg_destfile[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME+ 1];
            UI8_T       arg_srcfile[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME+ 1];
            UI32_T      arg_file_type;
            void        *arg_cookie;
            UI32_T      arg_ipc_message_q;
            void        (*arg_callback) (void *cookie, UI32_T status);
        } arg_grp_bool_localtounit;

        struct
	    {
            XFER_MGR_TftpMgtEntry_T     arg_tftp_mgt_entry_Info;
	    } arg_grp_bool_gettftpmgtentry;

        struct
	    {
            UI32_T      arg_tftp_file_type;
	    } arg_grp_ui32_settftpfiletype;

        struct
	    {
            UI8_T       arg_tftp_src_file[MAXSIZE_tftpSrcFile+ 1];
	    } arg_grp_ui32_settftpsrcfile;

        struct
	    {
            UI8_T       arg_tftp_des_file[MAXSIZE_tftpDestFile+ 1];
	    } arg_grp_ui32_settftpdestfile;

        struct
	    {
            L_INET_AddrIp_T   arg_tftp_server;
	    } arg_grp_ui32_settftpserver;

        struct
        {
            XFER_MGR_UserInfo_T  arg_user_info;
            UI32_T               arg_tftp_active;
        } arg_grp_ui32_settftpactive;

        struct
	    {
            UI8_T       arg_file_name_p[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME+ 1];
            UI8_T      arg_software_version_p[SYS_ADPT_FW_VER_STR_LEN+1];
	    } arg_grp_bool_getswversion;

        struct
	    {
            UI8_T       arg_tftp_error_msg[MAXSIZE_fileCopyTftpErrMsg+1];
	    } arg_grp_bool_gettftperrormsg;

        struct
	    {
            XFER_MGR_FileCopyMgt_T      arg_file_copy_mgt;
	    } arg_grp_bool_getfilecopymgtentry;

        struct
	    {
            UI32_T      arg_src_oper_type;
	    } arg_grp_bool_setfilecopysrcopertype;

        struct
	    {
            UI8_T       arg_src_file_name_p[MAXSIZE_fileCopySrcFileName+ 1];
	    } arg_grp_bool_setfilecopysrcfilename;

        struct
	    {
            UI32_T      arg_dest_oper_type;
	    } arg_grp_bool_setfilecopydestopertype;

        struct
	    {
            UI8_T       arg_dest_file_name_p[MAXSIZE_fileCopyDestFileName+ 1];
	    } arg_grp_bool_setfilecopydestfilename;

        struct
	    {
            UI32_T      arg_file_type;
	    } arg_grp_bool_setfilecopyfiletype;

        struct
	    {
            L_INET_AddrIp_T   arg_tftp_server;
	    } arg_grp_bool_setfilecopytftpserver;

		struct
	    {
            UI32_T      arg_key_type;
	    } arg_grp_bool_setpublickeytype;

		struct
	    {
            char       arg_username[SYS_ADPT_MAX_USER_NAME_LEN + 1];
	    } arg_grp_bool_setcopyusername;

        struct
	    {
            UI32_T      arg_unit;
	    } arg_grp_bool_setfilecopyunit;

        struct
        {
            XFER_MGR_UserInfo_T  arg_user_info;
            UI32_T               arg_action;
            void                 *arg_cookie;
            UI32_T               arg_ipc_message_q;
            void                 (*arg_callback) (void *cookie, UI32_T status);
        } arg_grp_bool_setfilecopyaction;

        struct
	    {
            UI8_T       arg_file_name[MAXSIZE_tftpSrcFile + 1];
	    } arg_grp_bool_getdualstartupcfgfilename;

        struct
	    {
            UI8_T       arg_unit_list[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
            UI8_T       arg_destfile[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME+ 1];
            UI8_T       arg_srcfile[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME+ 1];
            UI32_T      arg_file_type;
            BOOL_T      arg_is_next_boot;
            void        *arg_cookie;
            UI32_T      arg_ipc_message_q;
            void        (*arg_callback) (void *cookie, UI32_T status);
	    } arg_grp_bool_autodownload;

        struct
	    {
            UI32_T                      arg_unit_id;
            XFER_MGR_Auto_Download_T    arg_download_status;
	    } arg_grp_bool_getautodownload_status;

        struct
	    {
            UI32_T                      arg_unit_id;
            XFER_MGR_Auto_Download_T    arg_download_status;
	    } arg_grp_bool_getnextautodownload_status;

        struct
	    {
            UI8_T       arg_file_name[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME+ 1];
            void        *arg_cookie;
            UI32_T      arg_ipc_message_q;
            void        (*arg_callback) (void *cookie, UI32_T status);
	    } arg_grp_bool_autoconfigtounit;

        struct
	    {
            BOOL_T      arg_checkimagetype;
	    } arg_grp_void_setcheckimagetype;

        struct
	    {
            UI32_T      arg_file_type;
            UI8_T       arg_file_name[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME+ 1];
            void        *arg_cookie;
            UI32_T      arg_ipc_message_q;
            void        (*arg_callback) (void *cookie, UI32_T status);
	    } arg_grp_bool_setstartupfilename;

        struct
        {
            XFER_MGR_UserInfo_T arg_user_info;
            UI8_T               arg_dest_file_name[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME+ 1];
            FS_File_Type_T      arg_file_type;
            I32_T               arg_x_buf_offset;
            UI32_T              arg_length;
            void                *arg_cookie;
            UI32_T              arg_ipc_message_q;
            void                (*arg_callback) (void *cookie, UI32_T status);
        } arg_grp_bool_writefile;

        struct
        {
            L_INET_AddrIp_T     arg_server_ip;
        } arg_grp_bool_setserverinetaddress;

        struct
        {
            UI8_T       arg_username[MAXSIZE_fileCopyServerUserName + 1];
        } arg_grp_bool_setserverusername;

        struct
        {
            UI8_T       arg_password[MAXSIZE_fileCopyServerPassword + 1];
        } arg_grp_bool_setserverpassword;

#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
        UI32_T  download_Length;
        UI8_T   oam_portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
#endif
#endif
#endif
#endif
        UI32_T  file_copy_action_flag;
        UI32_T  file_copy_status_flag;
        UI32_T  partial_provision_status;
        UI32_T  tftp_retry_times;
        UI32_T  tftp_timeout;
        UI32_T  auto_upgrade_status;
        UI32_T  auto_upgrade_reload_status;
        char    auto_upgrade_path[MAXSIZE_fileAutoUpgradeOpCodePath + 1];
        char    auto_upgrade_filename[MAXSIZE_fileAutoUpgradeOpCodeFileName + 1];
    } data; /* the argument(s) for the function corresponding to cmd */

} XFER_MGR_IpcMsg_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Init
 * -------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Xfer Task module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void XFER_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - XFER_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void XFER_MGR_Create_InterCSC_Relation(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CreateTask
 * -------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Xfer Task module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void XFER_MGR_CreateTask (void); /* called by XFER_INIT_CreateTask()*/

void XFER_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_EnterTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the XFER_MGR enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *--------------------------------------------------------------------------*/
void XFER_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the XFER_MGR enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------*/
void XFER_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the XFER_MGR enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *--------------------------------------------------------------------------*/
void XFER_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ProvisionComplete
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set provision complete flag to be true
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void XFER_MGR_ProvisionComplete(void);

SYS_TYPE_Stacking_Mode_T XFER_MGR_GetOperationMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFile
 *------------------------------------------------------------------------
 * FUNCTION: This function will remote file copy to local
 * INPUT   : user_info_p         -- user information entry
 *           publickey_username  -- username of public key
 *           publickey_type      -- public key type
 *           server_ip_p         -- remote server ip address
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
 * NOTE: 1.send XFER_MGR_COPYFILE message to MgrTask (Task under mgr)
 *       2.XFER_MGR_Mode_T:
 *         XFER_MGR_LOCAL_TO_REMOTE --- tftp upload
 *         XFER_MGR_REMOTE_TO_LOCAL --- tftp download
 *         XFER_MGR_LOCAL_TO_LOCAL  ---- local copy
 *
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_CopyFile(
    XFER_MGR_UserInfo_T *user_info_p,
    char *publickey_username,
    UI32_T publickey_type,
    L_INET_AddrIp_T *server_ip_p,
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
 * ROUTINE NAME - XFER_MGR_RemoteToStream
 *------------------------------------------------------------------------
 * FUNCTION: This function will download a file to the memory
 * INPUT   : user_info_p       -- user information entry
 *           server_type       -- remote server type
 *           server_ip_p       -- remote server ip address
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
 * NOTE    : send XFER_MGR_STREAMTOREMOTE message to MgrTask (Task under mgr)
 *           username/password are used for login to FTP server only.
 *------------------------------------------------------------------------
 */
BOOL_T
XFER_MGR_RemoteToStream(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    L_INET_AddrIp_T *server_ip_p,
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
* ROUTINE NAME - XFER_MGR_StreamToRemote_Check
*------------------------------------------------------------------------
* FUNCTION: This function will Upload the memory stream to remote file
* INPUT   : L_INET_AddrIp_T *server_p
*           UI8_T       *desfile
*           UI32_T      file_type
*           void        *cookie
*           void        (*callback) (void *cookie, UI32_T status)
* OUTPUT  : None
* RETURN  : TRUE: succes;FALSE: if check error
* NOTE: send XFER_MGR_STREAMTOREMOTE message to MgrTask (Task under mgr)
*------------------------------------------------------------------------*/
BOOL_T XFER_MGR_StreamToRemote_Check (L_INET_AddrIp_T *server_p,
                                UI8_T   *desfile,
                                UI32_T  file_type,
                                void    *cookie,
                                UI32_T  ipc_message_q,
                                void (*callback) (void *cookie, UI32_T status));

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_StreamToRemote_Write
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
 *           buffer_length  -- the length of data buffer
 *           offset         -- the offset of data buffer
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
XFER_MGR_StreamToRemote_Write(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    L_INET_AddrIp_T *server_p,
    UI8_T *username,
    UI8_T *password,
    UI8_T *desfile,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    UI32_T buffer_length,
    I32_T offset,
    void (*callback) (void *cookie, UI32_T status)
);


/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_StreamToRemote
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
 *           callback       -- the callback function to notify status
 *                             for caller; if caller cannot have a
 *                             callback function to know the status, it
 *                             can call XFER_MGR_GetFileCopyMgtEntry() to
 *                             get XFER_MGR_FileCopyMgt_T, then know the
 *                             status of the last copy procedure.
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE: send XFER_MGR_STREAMTOREMOTE message to MgrTask (Task under mgr)
 *------------------------------------------------------------------------
 */
BOOL_T
XFER_MGR_StreamToRemote(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    L_INET_AddrIp_T *server_p,
    UI8_T *username,
    UI8_T *password,
    UI8_T *desfile,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_StreamToLocal
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
XFER_MGR_StreamToLocal(
    XFER_MGR_UserInfo_T *user_info_p,
    UI8_T *filename,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_LocalToStream
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
XFER_MGR_LocalToStream(
    XFER_MGR_UserInfo_T *user_info_p,
    UI8_T *filename,
    UI8_T *xbuffer,
    UI32_T *xbuf_length,
    UI32_T stream_max_length
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ReadSystemConfig
 *------------------------------------------------------------------------
 * FUNCTION: This function will read startup cfg file to buffer
 * INPUT   : UI8_T*  xbuffer,
 *           UI32_T* xbuf_length
 * OUTPUT  : xbuf_length - Actual system configuration data length(byte count)
 * RETURN  : one of FS_RETURN_CODE_E
 * NOTE    : 1. synch. function
 *           2. pure code, so need not mutual protection
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_ReadSystemConfig(UI8_T *xbuffer, UI32_T *xbuf_length);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_UnitToLocal
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
XFER_MGR_UnitToLocal(
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
 * ROUTINE NAME - XFER_MGR_LocalToUnit
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
XFER_MGR_LocalToUnit(
    XFER_MGR_UserInfo_T *user_info_p,
    UI32_T unit_id,
    UI8_T *destfile,
    UI8_T *srcfile,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);


/* TFTPMGT API
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetTftpMgtEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the tftp info
 * INPUT   : tftp_mgt_entry_Info->tftp_file_type
 *           tftp_mgt_entry_Info->tftp_server
 *           tftp_mgt_entry_Info->tftp_file
 *           tftp_mgt_entry_Info->tftp_active
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetTftpMgtEntry(XFER_MGR_TftpMgtEntry_T *tftp_mgt_entry_Info);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpFileType
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_SetTftpFileType(UI32_T  tftp_file_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpSrcFile
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_SetTftpSrcFile(UI8_T  *tftp_src_file);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpDestFile
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_SetTftpDestFile(UI8_T  *tftp_dest_File);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpServer
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   : L_INET_AddrIp_T  *tftp_server
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_SetTftpServer(L_INET_AddrIp_T  *tftp_server);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpActive
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   : user_info_p    -- user information entry
 *           tftp_active    -- tftp server active
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T
XFER_MGR_SetTftpActive(
    XFER_MGR_UserInfo_T *user_info_p,
    UI32_T tftp_active
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetSWVersion
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the software version by file name
 * INPUT   : file_name_p - pointer of file name
 * OUTPUT  : software_version_p - pointer of software version
 * RETURN  : TRUE - success
 *           FALSE - fail
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetSWVersion(UI8_T *file_name_p, UI8_T *software_version_p);

/* FileCopyMgt API
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetTftpErrorMsg
 *------------------------------------------------------------------------
 * FUNCTION: Get tftp undefined message
 * INPUT   : None
 * OUTPUT  : tftp_error_msg
 * RETURN  : TRUE/FALSE
 * NOTE    : only for CLI to get tftp undefined message when copy status is
 *           XFER_MGR_FILE_COPY_TFTP_UNDEF_ERROR
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetTftpErrorMsg(UI8_T *tftp_error_msg);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetFileCopyMgtEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the file copy mgt info
 * INPUT   : None
 * OUTPUT  : file_copy_mgt
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetFileCopyMgtEntry(XFER_MGR_FileCopyMgt_T *file_copy_mgt);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopySrcOperType
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
 *           also be changed by XFER_MGR_SetFileCopySrcFileName.
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopySrcOperType(UI32_T src_oper_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopySrcFileName
 *------------------------------------------------------------------------
 * FUNCTION: Set the source file name
 * INPUT   : UI8_T  *src_file_name_p  -- the starting address of source file name
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if the source operation type is VAL_fileCopySrcOperType_runningCfg(2)
 *           or VAL_fileCopySrcOperType_startUpCfg(3), this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopySrcFileName(UI8_T *src_file_name_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyDestOperType
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
 *           also be changed by XFER_MGR_SetFileCopyDestFileName.
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopyDestOperType(UI32_T dest_oper_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyDestFileName
 *------------------------------------------------------------------------
 * FUNCTION: Set the destination file name
 * INPUT   : UI8_T  *dest_file_name_p   -- the starting address of destination file name
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if the destination operation type is VAL_fileCopyDestOperType_runningCfg(2)
 *           or VAL_fileCopyDestOperType_startUpCfg(3), this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopyDestFileName(UI8_T *dest_file_name_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyFileType
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
BOOL_T XFER_MGR_SetFileCopyFileType(UI32_T file_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyTftpServer
 *------------------------------------------------------------------------
 * FUNCTION: Set the IP address of the TFTP server
 * INPUT   : UI32_T  tftp_server    -- the TFTP server IP address
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If neither source operation type nor destination operation type
 *           is tftp(4), this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopyTftpServer(L_INET_AddrIp_T  *tftp_server);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyUnit
 *------------------------------------------------------------------------
 * FUNCTION: Set the unit id
 * INPUT   : UI32_T  unit    -- the unit id
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopyUnit(UI32_T unit);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyAction
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
XFER_MGR_SetFileCopyAction(
    XFER_MGR_UserInfo_T *user_info_p,
    UI32_T action,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

#if(SYS_CPNT_3COM_TWO_OPCODES == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyTftpToStartupOpCode
 *------------------------------------------------------------------------
 * FUNCTION:  Copy remote op code with a destination file name which uses
 *            2 fixed op code file names in turn and save this file to startup.
 * INPUT   :  server_ip
 *            *srcfile
 *            cookie                                     -> 0, if callback is not necessary
 *            (*callback) (void *cookie, UI32_T status) -> 0, if callback is not necessary
 * OUTPUT  : None
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    : only for 3Com project
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_CopyTftpToStartupOpCode(UI32_T  server_ip,
                                        UI8_T   *srcfile,
                                        void    *cookie,
                                        UI32_T  ipc_message_q,
                                        void    (*callback) (void *cookie, UI32_T status));

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpToStartupOpCodeAction
 *------------------------------------------------------------------------
 * FUNCTION: Start the copy operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : only for 3Com project
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetTftpToStartupOpCodeAction(void);
#endif /* #if(SYS_CPNT_3COM_TWO_OPCODES == TRUE) */

#if(SYS_CPNT_CFGDB == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetLastSWUpgradeIp
 *------------------------------------------------------------------------
 * FUNCTION: Get the last ip for software upgrade
 * INPUT   : None
 * OUTPUT  : UI32_T  *tftp_server_ip
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetLastSWUpgradeIp(UI32_T *tftp_server_ip_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetLastSWUpgradeFileName
 *------------------------------------------------------------------------
 * FUNCTION: Get the last filename for software upgrade
 * INPUT   : None
 * OUTPUT  : UI8_T *file_name_p
 * RETURN  : TRUE/FALSE
 * NOTE    : file name length is restricted between MINSIZE_tftpSrcFile and
 *           MAXSIZE_tftpSrcFile
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetLastSWUpgradeFileName(UI8_T *file_name_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetLastSavingConfigIp
 *------------------------------------------------------------------------
 * FUNCTION: Get the last ip for saving config
 * INPUT   : None
 * OUTPUT  : UI32_T  *tftp_server_ip
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetLastSavingConfigIp(UI32_T *tftp_server_ip_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetLastSavingConfigFileName
 *------------------------------------------------------------------------
 * FUNCTION: Get the last filename for saving config
 * INPUT   : None
 * OUTPUT  : UI8_T *file_name_p
 * RETURN  : TRUE/FALSE
 * NOTE    : file name length is restricted between MINSIZE_tftpDestFile and
 *           MAXSIZE_tftpDestFile
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetLastSavingConfigFileName(UI8_T *file_name_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetLastRestoringConfigIp
 *------------------------------------------------------------------------
 * FUNCTION: Get the last ip for restoring config
 * INPUT   : None
 * OUTPUT  : UI32_T  *tftp_server_ip
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetLastRestoringConfigIp(UI32_T *tftp_server_ip_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetLastRestoringConfigFileName
 *------------------------------------------------------------------------
 * FUNCTION: Get the last filename for restoring config
 * INPUT   : None
 * OUTPUT  : UI8_T *file_name_p
 * RETURN  : TRUE/FALSE
 * NOTE    : file name length is restricted between MINSIZE_tftpSrcFile and
 *           MAXSIZE_tftpSrcFile
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetLastRestoringConfigFileName(UI8_T *file_name_p);
#endif /* #if(SYS_CPNT_CFGDB == TRUE) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_AbortTftp
 *------------------------------------------------------------------------
 * FUNCTION: abort what tftp is doing
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : only effect durring tftp is in progress
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_AbortTftp();

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetUsingStatus
 *------------------------------------------------------------------------
 * FUNCTION: Get the using status of XFER
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : XFER_MGR_UsingStatus_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
XFER_MGR_UsingStatus_T XFER_MGR_GetUsingStatus();

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyServerInetAddress
 *------------------------------------------------------------------------
 * FUNCTION: Set the inet address of the server.
 * INPUT   : server
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_SetFileCopyServerInetAddress(L_INET_AddrIp_T *server_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyServerUserName
 *------------------------------------------------------------------------
 * FUNCTION: Set the user name.
 * INPUT   : username
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_SetFileCopyServerUserName(UI8_T *username);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyServerPassword
 *------------------------------------------------------------------------
 * FUNCTION: Set the password.
 * INPUT   : password
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_SetFileCopyServerPassword(UI8_T *password);

#if (TRUE == SYS_CPNT_DBSYNC_TXT)

/*----------------------------------------------------------------------------------+
 * ROUTINE NAME - XFER_MGR_Get_Slave_Unit_Dual_Startup_Cfg_FileName                 |
 *----------------------------------------------------------------------------------+
 * FUNCTION:                                                                        |
 *          Get 2 fixed startup config file names in turn                           |
 * INPUT   :                                                                        |
 *          unit_id     :   Slave unit ID                                           |
 *          file_name   :   output filename buffer                                  |
 * OUTPUT  :                                                                        |
 *          file_name   :   next startup file( have not been set as startup yet )   |
 * RETURN  :                                                                        |
 *          TRUE        :   success                                                 |
 *          FALSE       :   if check error                                          |
 * NOTE    :None                                                                    |
 *---------------------------------------------------------------------------------*/
BOOL_T XFER_MGR_Get_Slave_Unit_Dual_Startup_Cfg_FileName( UI32_T unit_id, UI8_T * file_name );

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetDualStartupCfgFileName
 *------------------------------------------------------------------------
 * FUNCTION: Get 2 fixed startup config file names in turn
 * INPUT   : None
 * OUTPUT  : file_name
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    : only for 3Com project
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetDualStartupCfgFileName(UI8_T *file_name);

#endif /* #if (TRUE == SYS_CPNT_DBSYNC_TXT) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Get TFTP retry times
 * INPUT   : None
 * OUTPUT  : UI32_T *retry_times_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetTftpRetryTimes(UI32_T *retry_times_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetRunningTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Get TFTP retry times
 * INPUT   : None
 * OUTPUT  : UI32_T *retry_times_p
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_GetRunningTftpRetryTimes(UI32_T *retry_times_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Set TFTP retry times
 * INPUT   : UI32_T retry_times
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Range -- XFER_MGR_MIN_TFTP_RETRY_TIMES~XFER_MGR_MAX_TFTP_RETRY_TIMES
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetTftpRetryTimes(UI32_T retry_times);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetTftpTimeout
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves TFTP timeout value in seconds before retry
 * INPUT   : None
 * OUTPUT  : timeout_p  - timeout value in seconds
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_GetTftpTimeout(UI32_T *timeout_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetRunningTftpTimeout
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
UI32_T XFER_MGR_GetRunningTftpTimeout(UI32_T *timeout_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Set TFTP timeout value in seconds before retry
 * INPUT   : timeout    - timeout value in seconds
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_SetTftpTimeout(UI32_T timeout);

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetAutoOpCodeUpgradeStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set auto image upgrade status
 * INPUT   : status - auto image upgrade status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_SetAutoOpCodeUpgradeStatus(UI32_T status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetAutoOpCodeUpgradeStatus
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade status
 * INPUT   : None
 * OUTPUT  : status_p - auto image upgrade status
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_GetAutoOpCodeUpgradeStatus(UI32_T *status_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetRunningAutoOpCodeUpgradeStatus
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
SYS_TYPE_Get_Running_Cfg_T XFER_MGR_GetRunningAutoOpCodeUpgradeStatus(UI32_T *status_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetAutoOpCodeUpgradeReloadStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set auto image upgrade reload status
 * INPUT   : status - auto image upgrade reload status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_SetAutoOpCodeUpgradeReloadStatus(UI32_T status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetAutoOpCodeUpgradeReloadStatus
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade reload status
 * INPUT   : None
 * OUTPUT  : status_p - auto image upgrade reload status
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_GetAutoOpCodeUpgradeReloadStatus(UI32_T *status_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetRunningAutoOpCodeUpgradeReloadStatus
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
SYS_TYPE_Get_Running_Cfg_T XFER_MGR_GetRunningAutoOpCodeUpgradeReloadStatus(UI32_T *status_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetAutoOpCodeUpgradePath
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
BOOL_T XFER_MGR_SetAutoOpCodeUpgradePath(const char *path_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetAutoOpCodeUpgradePath
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade search path
 * INPUT   : None
 * OUTPUT  : path_p - search path
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_GetAutoOpCodeUpgradePath(char *path_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetRunningAutoOpCodeUpgradePath
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
SYS_TYPE_Get_Running_Cfg_T XFER_MGR_GetRunningAutoOpCodeUpgradePath(char *path_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetAutoOpCodeUpgradeFileName
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves auto image upgrade search file name
 * INPUT   : None
 * OUTPUT  : filename_p - search image's file name
 * RETURN  : TRUE/FALSE
 * NOTE    : The file name is read-only
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_GetAutoOpCodeUpgradeFileName(char *filename_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_NETCFG_RifUp_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: If a IP address have be configured to the management VLAN,
 *           we will receive the rif up event from netcfg.
 * INPUT   : UI32_T ifindex, L_INET_AddrIp_T *addr_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : To check the new image for upgrade if we have received the event
 *           and the CLI provision is completed. Because if the CLI provision is
 *           not completed yet, the socket library is not available.
 *           All CSC are available after provision complete.
 *------------------------------------------------------------------------
 */
void XFER_MGR_NETCFG_RifUp_CallBack(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */

/*------------------------------------------------------------------------
* ROUTINE NAME - XFER_MGR_AutoDownLoad
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
BOOL_T XFER_MGR_AutoDownLoad(UI8_T      *unit_list,
                             UI8_T      *destfile,
                             UI8_T      *srcfile,
                             UI32_T     file_type,
                             BOOL_T     is_next_boot,
                             void       *cookie,
                             UI32_T     ipc_message_q,
                             void       (*callback) (void *cookie, UI32_T status));

/*------------------------------------------------------------------------
* ROUTINE NAME - XFER_MGR_AutoConfigToUnit
*------------------------------------------------------------------------
* FUNCTION: This function will send XFER_MGR_CONFIGTOUNIT message to MgrTask (Task under mgr)
* INPUT   : UI8_T       *file_name
*           void        *cookie,
*           void        (*callback) (void *cookie, UI32_T status)
* OUTPUT  : None
* RETURN  : TRUE: succes;FALSE: if check error
* NOTE:
*------------------------------------------------------------------------*/
BOOL_T XFER_MGR_AutoConfigToUnit(UI8_T      *file_name,
                                 void       *cookie,
                                 UI32_T     ipc_message_q,
                                 void       (*callback) (void *cookie, UI32_T status));

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetAutoDownLoad_Status
 *------------------------------------------------------------------------
 * FUNCTION:  Get auto download status
 * INPUT   : unit_id -> unit id
 * OUTPUT  : *auto_download_status
 * RETURN  : TRUE: succes;FALSE: fail
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetAutoDownLoad_Status(UI32_T unit_id, XFER_MGR_Auto_Download_T *auto_download_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetNextAutoDownLoad_Status
 *------------------------------------------------------------------------
 * FUNCTION:  Get auto download status
 * INPUT   : *unit_id -> unit id
 * OUTPUT  : *auto_download_status
 * RETURN  : TRUE: succes;FALSE: fail
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetNextAutoDownLoad_Status(UI32_T *unit_id, XFER_MGR_Auto_Download_T *auto_download_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XFER_MGR_GetFileCopyStatusString
 *-------------------------------------------------------------------------
 * PURPOSE  : Get descriptive status.
 * INPUT    : XFER_MGR_FileCopyStatus_T     status
 * OUTPUT   : UI8_T                         *status_string
 * RETURN   : None
 * NOTE     : the buffer size of status string should be MAXSIZE_fileCopyTftpErrMsg
 *-------------------------------------------------------------------------*/
void XFER_MGR_GetFileCopyStatusString(XFER_MGR_FileCopyStatus_T status, UI8_T *status_string);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XFER_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void XFER_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XFER_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void XFER_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetStartupFilename
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
BOOL_T XFER_MGR_SetStartupFilename(UI32_T   file_type,
                                   UI8_T    *file_name,
                                   void     *cookie,
                                   UI32_T   ipc_message_q,
                                   void     (*callback) (void *cookie, UI32_T status));

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_WriteFile
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
 *                              can call XFER_MGR_GetFileCopyMgtEntry() to
 *                              get XFER_MGR_FileCopyMgt_T, then know the
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
XFER_MGR_WriteFile(
    XFER_MGR_UserInfo_T *user_info_p,
    UI8_T *dest_file_name,
    FS_File_Type_T file_type,
    UI8_T *x_buf,
    UI32_T length,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

#if defined(JBOS)
void XFER_MGR_SetCheckImageType(BOOL_T CheckImageType);
#endif

/* for copy running to file */
BOOL_T XFER_MGR_StreamToLocal_check (UI8_T   *filename,
                               UI32_T  file_type,
                               void   *cookie,
                               UI8_T  *buffer,
                               void    (*callback) (void *cookie, UI32_T status));

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_StreamToLocal_write
 *------------------------------------------------------------------------
 * FUNCTION: This function will save memory stream to local file
 * INPUT   : user_info_p    -- user information entry
 *           filename       -- local file name
 *           file_type      -- file type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
 *           ipc_message_q  -- the key of CSC group message queue
 *           buffer         -- data buffer use to load file
 *           buffer_length  -- the length of data buffer
 *           offset         -- the offset of data buffer
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
XFER_MGR_StreamToLocal_write(
    XFER_MGR_UserInfo_T *user_info_p,
    UI8_T *filename,
    UI32_T file_type,
    void *cookie,
    UI32_T ipc_message_q,
    UI8_T *buffer,
    UI32_T buffer_length,
    I32_T offset,
    void (*callback) (void *cookie, UI32_T status)
);

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
BOOL_T XFER_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetInfoDownloadLength
 * ------------------------------------------------------------------------
 * FUNCTION : This function will return the length for tftp download
 * INPUT    : None
 * OUTPUT   : *out_len
 * RETURN   : None
 * NOTE     : for OAM remote FW upgrade requirment.
 * ------------------------------------------------------------------------
 */
void XFER_MGR_GetInfoDownloadLength(UI32_T *out_len);
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
BOOL_T XFER_MGR_SetFileCopyActionFlag(UI32_T action);

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
BOOL_T XFER_MGR_SetFileCopyStatusFlag(UI32_T status);

#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyOamPorts
 *------------------------------------------------------------------------
 * FUNCTION: Set the port list for OAM
 * INPUT   : *in_port_list_p - the set of ifindex in port bitmap format
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if the dest operation type != VAL_fileCopyDestOperType_oamRemote
 *           ,this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopyOamPorts(UI8_T *in_portlist_p);
#endif
#endif
#endif
#endif


#endif /* end _XFER_MGR_H_ */


