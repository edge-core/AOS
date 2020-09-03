/* Project Name: Mercury
 * Module Name : XFER_MGR.C
 * Purpose     :
 *
 * History :
 *          Date        Modifier        Reason
 *          2001/10/11  BECK CHEN       Create this file
 *          2002/12/05  Erica Li        Add XFER_MGR_GetFileCopyMgtEntry()
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
 *          2003/05/02  Erica Li        Change sequence of include files
 *          2003/07/31  Erica Li        Add XFER_MGR_AbortTftp()
 *          2003/08/06  Erica Li        Add XFER_MGR_GetDualStartupCfgFileName()
 *          2003/08/14  Erica Li        Add XFER_MGR_GetTftpRetryTimes()
 *                                      Add XFER_MGR_GetRunningTftpRetryTimes()
 *                                      Add XFER_MGR_SetTftpRetryTimes()
 *          2003/12/05  Erica Li        Check project ID by new spec:
 *                                      Accept Firmware Image if project ID the same or
 *                                      if family ID the same and unversal bit is set.
 *          2003/12/10  Erica Li        download multiple image to mainboard and
 *                                      split the second image to option module
 *          2004/05/06  Erica Li        Add the retry mechanism of auto syncronization
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 *
 * Note    :
 */

 /* INCLUDE FILE    DECLARATIONS
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "xfer_mgr.h"
#include "xfer_buf_mgr.h"
#include "xfer_dnld.h"
#include "fs.h"
#include "fs_type.h"
#include "xfer_type.h"
#include "l_mm.h"
#include "l_stdlib.h"
#include "leaf_es3626a.h"
#include "syslog_type.h"
#include "syslog_pmgr.h"
#include "sys_imghdr.h"
#include "l_math.h"
#include "l_threadgrp.h"
#include "stktplg_pom.h"
#include "stkctrl_pmgr.h"
#include "backdoor_mgr.h"
#include "sys_dfltcfg.h"
#include "sys_module.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "sysdrv.h"
#include "ip_lib.h"
/* for reading running config
 */
#include "uc_mgr.h"
#include "cli_mgr.h"
#include "cli_pmgr.h"
#if (SYS_CPNT_DBSYNC_TXT ==TRUE)
#include "dbsync_txt_mgr.h"
#endif

#include "xfer_om.h"
#include "xfer_om_private.h"

#include "sys_callback_mgr.h"
#include "xfer_proc_comm.h"
#include "buffer_mgr.h"
#include "keygen_type.h"
#include "userauth.h"
#include "userauth_pmgr.h"
#include "sshd_pmgr.h"
#include "cmdftp.h"

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
#include "netcfg_type.h"
#include "sys_time.h"
#include "trap_event.h"
#include "ipal_types.h"
#include "ipal_route.h"
#if (SYS_CPNT_TRAPMGMT == TRUE)
#include "trap_pmgr.h"
#else
#include "snmp_pmgr.h"
#endif
#endif  /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */
#include "ini.h"

#if (SYS_CPNT_ONIE_SUPPORT==FALSE)
/* The ONIE uboot header file image.h does not contain ih_ver and ih_pid in
 * image_t which are added by Accton. Only Accton customized uboot contains
 * these two fields in image_t.
 */
/* The following three typedef is used in image.h
 */
typedef	unsigned char	uint8_t;
typedef	unsigned short	uint16_t;
typedef	unsigned int	uint32_t;

#include "image.h" /* uboot image header, include for image_header_t */
#endif

#if (SYS_CPNT_SINGLE_RUNTIME_IMAGE==TRUE)
#include "ams_part.h" /* for TYPE_RUNTIME */
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_CONFDB == TRUE)
#include "confdb.h"
#endif /* SYS_CPNT_CONFDB */

#include "l_charset.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define XFER_MGR_MSG_Q_LEN   256
#define XFER_MGR_EVENT_MSG                 BIT_1
#define XFER_MGR_EVENT_ENTER_TRANSITION    BIT_2
#define XFER_MGR_AUTO_SYNC_NUMBER_OF_RETRIES    3
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
#define XFER_MGR_AUTO_UPGRADE_OPCODE_DEFAULT_FILENAME "op1.swi"
#define XFER_MGR_AUTO_UPGRADE_OPCODE_SECOND_FILENAME  "op2.swi"
#else
#define XFER_MGR_AUTO_UPGRADE_OPCODE_DEFAULT_FILENAME "op1.bix"
#define XFER_MGR_AUTO_UPGRADE_OPCODE_SECOND_FILENAME  "op2.bix"
#endif

#define USB_PATH_LENGTH (512)
#define USB_FILEOPER_MGR_BLOCKSIZE (512 * 2 * 32)
#define USB_MAXSIZE_DestFile (128)

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
#define AOS_ONIE_GET_INSTALLER_VER_SCRIPT_FILENAME "/etc/aos_get_installer_ver.sh"
#define AOS_ONIE_VERIFY_INSTALLER_SCRIPT_FILENAME "/etc/aos_verify_installer.sh"
#define REDIRECT_STDOUT_STDERR_TO_NULL_CMD "1>/dev/null 2>&1"
#define SCRIPT_ERROR_STR "Error!"
#define AOS_ONIE_INSTALLER_MAX_IMG_HEADER_SIZE (8*1024)
#define XFER_MGR_CSC_NAME "xfer_mgr"
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    XFER_MGR_COPYFILE = 0,
    XFER_MGR_REMOTETOSTREAM,
    XFER_MGR_STREAMTOLOCAL,
    XFER_MGR_STREAMTOREMOTE,
    XFER_MGR_COPYUNITFILE,
    XFER_MGR_COPYFILEUNIT,
    XFER_MGR_COPYTOALLUNIT,
    XFER_MGR_WRITEFILE,
    XFER_MGR_UPGRADEFILE,
    XFER_MGR_CONFIGTOUNIT,
}XFER_MGR_MTYPE_T;

typedef enum
{
  XFER_MGR_URL_TOKEN_SCHEME = 0,
  XFER_MGR_URL_TOKEN_USER,
  XFER_MGR_URL_TOKEN_PASSWORD,
  XFER_MGR_URL_TOKEN_HOST,
  XFER_MGR_URL_TOKEN_PORT,
  XFER_MGR_URL_TOKEN_PATHNAME,
  XFER_MGR_URL_TOKEN_MAX
} XFER_MGR_URL_TOKEN;

typedef struct
{
    XFER_MGR_UserInfo_T      user_info;
    L_INET_AddrIp_T          server_ip;                                  /*tftp server ip*/
    UI32_T                   unit_id;                                    /*unit id, erica 09/02/02 for stackable system*/
    UI32_T                   status;
    UI32_T                   src_oper_type;
    UI32_T                   dest_oper_type;

    UI32_T                   publickey_type;
    char                     publickey_username[SYS_ADPT_MAX_USER_NAME_LEN + 1]; /* username to download public key from remote server */

    UI8_T                    unit_list[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI8_T                    destfile [MAXSIZE_fileCopyDestFileName + 1];   /* tftp file name */
    UI8_T                    srcfile [MAXSIZE_fileCopySrcFileName + 1];

    UI8_T                    username[MAXSIZE_fileCopyServerUserName + 1];   /* username */
    UI8_T                    password[MAXSIZE_fileCopyServerPassword + 1];   /* password */

    UI32_T                   file_type;
    UI8_T                    *buf;
    UI32_T                   length;                                     /* buf length or data length */
    I32_T                    offset;
    XFER_MGR_Mode_T          mode;
    XFER_MGR_RemoteServer_T  server_type;
    BOOL_T                   is_next_boot;
    BOOL_T                   auto_assign_dest_runtime_filename;  /* if TRUE, the destination runtime file name will be assigned
                                                         * automatically.
                                                         * this flag is take effect just on mode is XFER_MGR_REMOTE_TO_LOCAL
                                                         * and file_type is FS_TYPE_OPCODE_FILE_TYPE only.
                                                         */
    void                     *cookie;
    UI32_T                   ipc_message_q;
    void                     (*callback) (void *cookie, UI32_T status);
} Mtext_T;

typedef struct
{
    XFER_MGR_MTYPE_T    mtype;
    Mtext_T             *mtext;
    UI32_T              reserved[2];
} XFER_Msg_T;

typedef struct
{
    XFER_DNLD_TFTPStatus_T  tftp_status;
    XFER_DNLD_TFTPStatus_T  current_tftp_status;        /* to check whether the tftp transaction is end */
    UI32_T                  tftp_download_length;
    BOOL_T                  is_busy;
    BOOL_T                  is_next_boot;
    BOOL_T                  is_end_with_provision;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
    BOOL_T                  is_partial_provision;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

} Info_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void     XFER_MGR_Main(void);
static void     XFER_MGR_InitTftpMgrEntry(void);
#if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
static BOOL_T   XFER_MGR_IsCheckProjectId(void);
#endif
static BOOL_T   XFER_MGR_Transaction_Begin(BOOL_T immediately, BOOL_T *busy_p);
static void     XFER_MGR_Transaction_End(BOOL_T clear_busy_flag);
static void     XFER_MGR_TftpDnld_CallBack( UI32_T status,UI32_T download_length);
static void     XFER_MGR_CallBack ( Mtext_T *mtext, XFER_MGR_FileCopyStatus_T  status);
static void     XFER_MGR_CopyFile_Syn (Mtext_T *mtext);
static void     XFER_MGR_RemoteToStream_Syn (Mtext_T *mtext);
static void     XFER_MGR_StreamToRemote_Syn (Mtext_T *mtext);
static void     XFER_MGR_StreamToLocal_Syn (Mtext_T *mtext);
static void     XFER_MGR_WriteFile_Syn (Mtext_T *mtext);
static void     XFER_MGR_UnitToLocal_Syn (Mtext_T *mtext);
static void     XFER_MGR_LocalToUnit_Syn (Mtext_T *mtext);
static void     XFER_MGR_AutoDownLoad_Syn (Mtext_T *mtext);
static void     XFER_MGR_AutoConfigToUnit_Syn (Mtext_T *mtext);
static BOOL_T XFER_MGR_CheckFreeSizeByFileType(FS_File_Type_T file_type,
    UI32_T unit_id, UI8_T *file_name, UI32_T file_size);
static BOOL_T   XFER_MGR_Delete_FileForCopy(UI32_T unit_id, UI32_T file_size);
static BOOL_T   XFER_MGR_Check_File_Version(UI8_T *unit_list, UI32_T file_type);
static BOOL_T   XFER_MGR_Check_File_Version_By_MText(UI8_T *unit_list, Mtext_T *mtext);
static BOOL_T   XFER_MGR_Get_SWFile_Version(UI32_T unit_id, UI8_T *version, UI32_T file_type);
static void     XFER_MGR_Delete_NonStartOPcode(UI32_T unit_id, UI8_T *dest_file_name_p, UI32_T file_type);
static void     XFER_MGR_InitAutoDownloadStatus(void);
static BOOL_T   XFER_MGR_CheckImageHeaderValidation (Mtext_T *mtext, UI32_T data_len);
static BOOL_T   XFER_MGR_IsCheckImageVersion(void);
static void     XFER_MGR_WaitForTransmitCompleted(void);
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
static BOOL_T XFER_MGR_GetSWVersionFromONIEInstallerFile(const char* installer_file_full_path_p, UI8_T *software_version_p);
static BOOL_T XFER_MGR_GetSWVersionFromONIEInstallerStream(const UI8_T* installer_stream_p, UI8_T *software_version_p);
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ConvertModeToDownloadFlag
 *------------------------------------------------------------------------
 * FUNCTION: The function checks the mode is download
 * INPUT   : mode - XFER_MGR_Mode_T
 * OUTPUT  : none
 * RETURN  : TRUE  - download mode
 *           FALSE - not download mode
 * NOTE    :
 *------------------------------------------------------------------------*/
static BOOL_T
XFER_MGR_ConvertModeToDownloadFlag(
    XFER_MGR_Mode_T mode
);

#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
static BOOL_T   XFER_MGR_CheckLoaderImageValidation (Mtext_T *mtext);
#endif

static BOOL_T   XFER_MGR_CopyUserInfoByServerType(XFER_MGR_RemoteServer_T server_type, const UI8_T *src_username, const UI8_T *src_password, UI8_T *dst_username, UI8_T *dst_password);

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
static BOOL_T   XFER_MGR_IsProvisionComplete();
static BOOL_T   XFER_MGR_GetRemoteImageSWVersion(UI32_T server_type, L_INET_AddrIp_T *server_ip_p, const UI8_T *username,
                                                 const UI8_T *password, const UI8_T *filepath, UI8_T *version);
static void     XFER_MGR_WaitForRoutingInterfaceReady(const L_INET_AddrIp_T *ipaddr, UI32_T timeout);

static BOOL_T   XFER_MGR_UpgradeFile(BOOL_T immediately);
static void     XFER_MGR_UpgradeFile_Syn (Mtext_T *mtext);
static BOOL_T   XFER_MGR_ParseUrl(char *src, char *list[XFER_MGR_URL_TOKEN_MAX]);
static BOOL_T   XFER_MGR_IsValidUrl(const char *src);
static void     XFER_MGR_SendAutoOpCodeUpgradeTrap(BOOL_T is_success);
static void     XFER_MGR_UpgradeFileProgress_CallBack(void *cookie, UI32_T status);
#endif  /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */

/* CallBack Functions for Backdoor information
 */
static void     XFER_MGR_BackdoorInfo_CallBack(void);
static void     XFER_MGR_Print_BackdoorHelp(void);

/* For fileCopyMgt
 */
static void     XFER_MGR_InitFileCopyMgtEntry(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFileFile
 *------------------------------------------------------------------------
 * FUNCTION: Copy from local file to local file
 * INPUT   : user_info_p    -- user information entry
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
 * NOTE    : If the type of file to copy is different from the source file's,
 *           will return FALSE.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyFileFile(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFileRunning
 *------------------------------------------------------------------------
 * FUNCTION: Copy from local file to running conifg
 * INPUT   : user_info_p    -- user information entry
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
 * NOTE    : After completed copy operation, will provision.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyFileRunning(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);


/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFileRemote
 *------------------------------------------------------------------------
 * FUNCTION: Copy from local file to remote
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
 * NOTE    : If the type of file to copy is different from the source file's,
 *           will return FALSE.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyFileRemote(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRunningFile
 *------------------------------------------------------------------------
 * FUNCTION: Copy from running config to local file
 * INPUT   : user_info_p    -- user information entry
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
static BOOL_T
XFER_MGR_CopyRunningFile(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRunningRemote
 *------------------------------------------------------------------------
 * FUNCTION: Copy from running config to tftp
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
static BOOL_T
XFER_MGR_CopyRunningRemote(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRemoteFile
 *------------------------------------------------------------------------
 * FUNCTION: Copy from remote to local file
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
static BOOL_T
XFER_MGR_CopyRemoteFile(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRemoteRunning
 *------------------------------------------------------------------------
 * FUNCTION: Copy from remote to running
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
 * NOTE    : After completed copy operation, will provision.
 *------------------------------------------------------------------------
 */
static
BOOL_T XFER_MGR_CopyRemoteRunning(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_AddRemoteRunning
 *------------------------------------------------------------------------
 * FUNCTION: Add remote config to running
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
 * NOTE    :
 *------------------------------------------------------------------------*/
static BOOL_T
XFER_MGR_AddRemoteRunning(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyUnitFile
 *------------------------------------------------------------------------
 * FUNCTION: Copy from other unit file to local file
 * INPUT   : user_info_p    -- user information entry
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
 * NOTE    : The type of file to copy must be FS_FILE_TYPE_CONFIG or FS_TYPE_OPCODE_FILE_TYPE.
 *           The destination file couldn't be factory default config.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyUnitFile(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFileUnit
 *------------------------------------------------------------------------
 * FUNCTION: Copy from local file to other unit
 * INPUT   : user_info_p    -- user information entry
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
 * NOTE    : The type of file to copy must be FS_FILE_TYPE_CONFIG or FS_TYPE_OPCODE_FILE_TYPE.
 *           The destination file couldn't be factory default config.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyFileUnit(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status)
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRemotePublickey
 *------------------------------------------------------------------------
 * FUNCTION: Copy TFTP publickey
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
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
static BOOL_T
XFER_MGR_CopyRemotePublickey(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    void (*callback) (void *cookie, UI32_T status)
);

static BOOL_T   XFER_MGR_IsDebugFlagOn(void);
static BOOL_T   XFER_MGR_IsValidServerIP(L_INET_AddrIp_T *server_p);
static BOOL_T   XFER_MGR_IsFileNumExceededByType(UI8_T *filename, UI32_T file_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ServerTypeToSrcOperType
 *------------------------------------------------------------------------
 * FUNCTION: convert server type to source operation type
 * INPUT   : server_type    -- remote server type
 * OUTPUT  : src_oper_type  -- source operation type
 * RETURN  : TRUE / FALSE
 * NOTE    : none
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_ServerTypeToSrcOperType(
    XFER_MGR_RemoteServer_T server_type,
    UI32_T *src_oper_type
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ServerTypeToDestOperType
 *------------------------------------------------------------------------
 * FUNCTION: convert server type to destnation operation type
 * INPUT   : server_type    -- remote server type
 * OUTPUT  : dest_oper_type -- destnation operation type
 * RETURN  : TRUE / FALSE
 * NOTE    : none
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_ServerTypeToDestOperType(
    XFER_MGR_RemoteServer_T server_type,
    UI32_T *dest_oper_type
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SendFileCopyTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send file copy trap
 * INPUT   : mtext - context of message of xfer
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_SendFileCopyTrap(
    Mtext_T *mtext
);

static int XFER_MGR_LicenseResHandler(void *output_p, const char *section_p, const char *name_p, const char *value_p);
static BOOL_T XFER_MGR_IsOpcodeVersionAllowByLicense(char *version_p);

/* LOCAL VARIABLES
 */
static SYSFUN_MsgQ_T             xfer_mgr_task_msgQ_id;
static UI32_T                    xfer_mgr_task_id;
static Info_T                    info;
static XFER_MGR_TftpMgtEntry_T   TftpMgrEntry;
static BOOL_T                    XFER_MGR_CheckProjectId = TRUE;
static BOOL_T                    XFER_MGR_CheckImageType = TRUE;
static BOOL_T                    XFER_MGR_CheckImageVersion = TRUE;
static BOOL_T                    is_transition_done;
static XFER_MGR_FileCopyMgt_T    file_copy_mgt_entry;
static BOOL_T                    is_file_copy_mgt_used = FALSE;
static XFER_MGR_Auto_Download_T  auto_download_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
static BOOL_T                    debug_flag = FALSE;
static BOOL_T                    onie_installer_debug_flag=FALSE;
static BOOL_T                    auto_download_sync_to_master = FALSE;
static BOOL_T                    is_provision_complete = FALSE;

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
/* Check for the new image at:
 * 1. receive the CLI provision complete event and xfer_mgr_rif_up is set.
 * 2. receive the rif up event and the CLI provision is completed now.
 * The check will be done only once.
 */

/* Change to  TRUE, if an IP address have be configured on managemnet VLAN.
 * Default value is FALSE.
 */
static BOOL_T                    xfer_mgr_rif_up;

/* Reocde the auto upgrade is done or not. The auto upgrade will be done only
 * once.
 */
static BOOL_T                    xfer_mgr_auto_upgrade_check;
#endif

/* declare variables used for Transition mode
 */
SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
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
void XFER_MGR_Init(void)
{
    /* create message queue. Wait forever if fail
     */
    XFER_DNLD_Init();
    XFER_OM_Init();

    is_transition_done = FALSE;
    XFER_MGR_InitTftpMgrEntry();
    XFER_MGR_InitFileCopyMgtEntry();
    XFER_MGR_InitAutoDownloadStatus();

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    xfer_mgr_rif_up = FALSE;
    xfer_mgr_auto_upgrade_check = TRUE;
#endif

}/* End of XFER_MGR_Init() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - XFER_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void XFER_MGR_Create_InterCSC_Relation(void)
{
    /* register the call back functions
     */
    XFER_DNLD_SetCallback(XFER_MGR_TftpDnld_CallBack);
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("TFTP", SYS_BLD_XFER_GROUP_IPCMSGQ_KEY, XFER_MGR_BackdoorInfo_CallBack);
} /* end of XFER_MGR_Create_InterCSC_Relation */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CreateTask
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create File transform Task. This function
 *           will be called by root.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void XFER_MGR_CreateTask(void)
{
     SYSLOG_OM_RecordOwnerInfo_T   owner_info;

    XFER_DNLD_CreateTask();

    if(SYSFUN_SpawnThread(SYS_BLD_XFER_GROUP_MGR_THREAD_PRIORITY,
                     SYS_BLD_XFER_GROUP_MGR_SCHED_POLICY,
                     SYS_BLD_XFER_CSC_THREAD_NAME,
                     SYS_BLD_TASK_COMMON_STACK_SIZE,
                     SYSFUN_TASK_NO_FP,
                     XFER_MGR_Main,
                     NULL,
                     &xfer_mgr_task_id)!=SYSFUN_OK)
    {
        owner_info.level =           SYSLOG_LEVEL_CRIT;
        owner_info.module_no =       SYS_MODULE_XFER;
        owner_info.function_no =     XFER_MGR_FUN_CREATE_TASK;
        owner_info.error_no = XFER_MGR_Create_Tasks_ErrNo;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, "XFER_MGR", 0, 0);
        return;
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_XFER, xfer_mgr_task_id, SYS_ADPT_XFER_SW_WATCHDOG_TIMER);
#endif

} /* End of XFER_MGR_CreateTask() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  Set transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void XFER_MGR_SetTransitionMode(void)
{
    XFER_DNLD_SetTransitionMode();

    /* MGR's set transition mode
     */
    SYSFUN_SET_TRANSITION_MODE();

    /* TASK's set transition mode
     */
    is_transition_done = FALSE;
    SYSFUN_SendEvent(xfer_mgr_task_id, XFER_MGR_EVENT_ENTER_TRANSITION);

}   /*  end of XFER_MGR_SetTransitionMode() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_EnterTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the XFER_MGR enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void XFER_MGR_EnterTransitionMode(void)
{
    XFER_DNLD_EnterTransitionMode();

    /* MGR's enter transition mode
     */
    SYSFUN_ENTER_TRANSITION_MODE();

    /* TASK's transition mode
     */
    /* Ask task to release all resources
     */
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);

    /* Original code
     */
    XFER_MGR_InitTftpMgrEntry();
    XFER_MGR_InitFileCopyMgtEntry();
    XFER_MGR_InitAutoDownloadStatus();

}/* End of XFER_MGR_EnterTransitionMode() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the XFER_MGR enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void XFER_MGR_EnterMasterMode(void)
{
    XFER_DNLD_EnterMasterMode();
    XFER_OM_EnterMasterMode();

    XFER_MGR_InitTftpMgrEntry();
    XFER_MGR_InitFileCopyMgtEntry();
    XFER_MGR_InitAutoDownloadStatus();

    SYSFUN_ENTER_MASTER_MODE();
} /* End of XFER_MGREnterMasterMode() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the XFER_MGR enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void XFER_MGR_EnterSlaveMode(void)
{
    XFER_DNLD_EnterSlaveMode();

    /* MGR's enter slave mode, TASK also ref. to this
     */
    SYSFUN_ENTER_SLAVE_MODE();

}/* End of XFER_MGR_EnterSlaveMode() */

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
void XFER_MGR_ProvisionComplete(void)
{
    is_provision_complete = TRUE; /* runtime provision has completed */

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    /* check the new image if the xfer_mgr_rif_up has be set during the CLI provision.
     */
    if (TRUE == xfer_mgr_rif_up)
    {
        XFER_MGR_UpgradeFile(FALSE);
    }
#endif /*#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)*/

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetOperationMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get current operation mode (master / slave / transition).
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : operation mode
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Stacking_Mode_T XFER_MGR_GetOperationMode(void)
{
   return SYSFUN_GET_CSC_OPERATING_MODE(); /*Charles*/
}/* End of XFER_MGR_GetOperationMode() */

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
    void (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    UI32_T  src_oper_type;
    UI32_T  dest_oper_type;
    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    BOOL_T set_is_busy_flag = FALSE;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    /* BODY
     */
    if (destfile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,destfile);
        return FALSE;
    }

    if (srcfile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,srcfile);
        return FALSE;
    }

    switch ( mode )
    {
        /* Upload */
        case XFER_MGR_LOCAL_TO_REMOTE:
        case XFER_MGR_STARTUP_TO_REMOTE:
            if(FALSE == XFER_MGR_IsValidServerIP(server_ip_p))
            {
                EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                                        EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
                return FALSE;
            }
            if (strlen((char *)destfile) > XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,destfile);
                return FALSE;
            }
            if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,srcfile);
                return FALSE;
            }
            if (FS_GenericFilenameCheck(srcfile, file_type) != FS_RETURN_OK)
            {
                return FALSE;
            }
            break;

        /* Download */
        case XFER_MGR_REMOTE_TO_LOCAL:
        case XFER_MGR_REMOTE_TO_STARTUP:
            if(FALSE == XFER_MGR_IsValidServerIP(server_ip_p))
            {
                EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                                        EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
                return FALSE;
            }

#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
            /* need to check destination filename, except upload loader
             */
            if(FS_FILE_TYPE_TOTAL != file_type)
#endif/* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */
            {
                if (strlen((char *)destfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
                {
                    EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                    EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,destfile);
                    return FALSE;
                }

                if (FS_GenericFilenameCheck(destfile, file_type) != FS_RETURN_OK)
                {
                    return FALSE;
                }
            }

            if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,srcfile);
                return FALSE;
            }

            /* the check item move to xxx_syn
             * check the number of operation files in flash
             */

            break;

        case XFER_MGR_LOCAL_TO_LOCAL:
        case XFER_MGR_LOCAL_TO_STARTUP:
        case XFER_MGR_STARTUP_TO_LOCAL:
            if (strlen((char *)destfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,destfile);
                return FALSE;
            }
            if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,srcfile);
                return FALSE;
            }
            if (FS_GenericFilenameCheck(srcfile, file_type) != FS_RETURN_OK)
            {
                return FALSE;
            }

            if (FS_GenericFilenameCheck(destfile, file_type) != FS_RETURN_OK)
            {
                return FALSE;
            }

            /* the check item move to xxx_syn
             * check the number of operation files in flash
             */

            break;

        case XFER_MGR_STARTUP_TO_STREAM:
            if (file_type != FS_FILE_TYPE_CONFIG)
            {
                return FALSE;
            }

            if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO, srcfile);
                return FALSE;
            }

            if (FS_GenericFilenameCheck(srcfile, file_type) != FS_RETURN_OK)
            {
                return FALSE;
            }
            break;

        case XFER_MGR_STREAM_TO_STARTUP:
            if (file_type != FS_FILE_TYPE_CONFIG)
            {
                return FALSE;
            }

            if (strlen((char *)destfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO, destfile);
                return FALSE;
            }

            if (FS_GenericFilenameCheck(destfile, file_type) != FS_RETURN_OK)
            {
                return FALSE;
            }
            break;

        /*copy usbdisk to file */
        case XFER_MGR_USBDISK_TO_LOCAL:
            if (strlen((char *)destfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,destfile);
                return FALSE;
            }

            /* Treat the file in USB disk as remote file, max size of file name for
             * remote file is XFER_TYPE_MAX_SIZE_OF_REMOTE_FILE_NAME.
             */
            if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_REMOTE_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,srcfile);
                return FALSE;
            }

            /* No need to check the validity of file name because USB disk is not
             * local FS.
             */
#if 0
            if (FS_GenericFilenameCheck(srcfile, file_type) != FS_RETURN_OK)
            {
                return FALSE;
            }
#endif

            if (FS_GenericFilenameCheck(destfile, file_type) != FS_RETURN_OK)
            {
                return FALSE;
            }

            break;
        case XFER_MGR_REMOTE_TO_STREAM:
        case XFER_MGR_REMOTE_TO_PUBLICKEY:
            if (FALSE == XFER_MGR_IsValidServerIP(server_ip_p))
            {
                EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                                        EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
                return FALSE;
            }

            if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
                EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO, srcfile);
                return FALSE;
            }
            break;
        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
            EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"copy mode");
            return FALSE;
            break;
    } /* End of switch */

     if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_COPYFILE)) )== NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if ( (mtext->buf = BUFFER_MGR_Allocate()) == 0 )
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free (mtext);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    strncpy(mtext->publickey_username, publickey_username, sizeof(mtext->publickey_username)-1);
    mtext->publickey_username[sizeof(mtext->publickey_username)-1] = '\0';
    mtext->publickey_type = publickey_type;
    mtext->mode =          mode;
    memcpy(&(mtext->server_ip), server_ip_p, sizeof(mtext->server_ip));
    mtext->cookie =        cookie;
    mtext->callback =      callback;
    mtext->file_type =     file_type;
    mtext->is_next_boot =  info.is_next_boot;
    mtext->server_type =   server_type;
    mtext->ipc_message_q = ipc_message_q;

    XFER_MGR_CopyUserInfoByServerType(server_type, username, password, mtext->username, mtext->password);

    strcpy((char *)mtext->destfile, (char *)destfile);
    strcpy((char *)mtext->srcfile, (char *)srcfile);

    src_oper_type = file_copy_mgt_entry.src_oper_type;
    dest_oper_type = file_copy_mgt_entry.dest_oper_type;

    if (FALSE == is_file_copy_mgt_used)
    {
        switch(mode)
        {
            case XFER_MGR_LOCAL_TO_REMOTE:
                src_oper_type = VAL_fileCopySrcOperType_file;

                if (FALSE == XFER_MGR_ServerTypeToDestOperType(server_type, &dest_oper_type))
                {
                    return FALSE;
                }
                break;
           case XFER_MGR_STARTUP_TO_REMOTE:
                src_oper_type = VAL_fileCopySrcOperType_startUpCfg;

                if (FALSE == XFER_MGR_ServerTypeToDestOperType(server_type, &dest_oper_type))
                {
                    return FALSE;
                }
                break;

            case XFER_MGR_REMOTE_TO_LOCAL:
                if (FALSE == XFER_MGR_ServerTypeToSrcOperType(server_type, &src_oper_type))
                {
                    return FALSE;
                }

                dest_oper_type = VAL_fileCopyDestOperType_file;
                break;

            case XFER_MGR_REMOTE_TO_STARTUP:
                if (FALSE == XFER_MGR_ServerTypeToSrcOperType(server_type, &src_oper_type))
                {
                    return FALSE;
                }

                dest_oper_type = VAL_fileCopyDestOperType_startUpCfg;
                break;

            case XFER_MGR_LOCAL_TO_LOCAL:
                src_oper_type = VAL_fileCopySrcOperType_file;
                dest_oper_type = VAL_fileCopyDestOperType_file;
                break;

            case XFER_MGR_LOCAL_TO_STARTUP:
                src_oper_type = VAL_fileCopySrcOperType_file;
                dest_oper_type = VAL_fileCopyDestOperType_startUpCfg;
                break;

            case XFER_MGR_STARTUP_TO_LOCAL:
                src_oper_type = VAL_fileCopySrcOperType_startUpCfg;
                dest_oper_type = VAL_fileCopyDestOperType_file;
                break;

            case XFER_MGR_STARTUP_TO_STREAM:
                src_oper_type = VAL_fileCopySrcOperType_startUpCfg;
                dest_oper_type = VAL_fileCopyDestOperType_runningCfg;
                break;

            case XFER_MGR_STREAM_TO_STARTUP:
                src_oper_type = VAL_fileCopySrcOperType_runningCfg;
                dest_oper_type = VAL_fileCopyDestOperType_startUpCfg;
                break;

            case XFER_MGR_USBDISK_TO_LOCAL:
                src_oper_type = VAL_fileCopySrcOperType_usb;
                dest_oper_type = VAL_fileCopyDestOperType_file;
                break;

            case XFER_MGR_REMOTE_TO_STREAM:
                if (FALSE == XFER_MGR_ServerTypeToSrcOperType(server_type, &src_oper_type))
                {
                    return FALSE;
                }

                dest_oper_type = VAL_fileCopyDestOperType_runningCfg;
                break;

            case XFER_MGR_REMOTE_TO_PUBLICKEY:
                if (FALSE == XFER_MGR_ServerTypeToSrcOperType(server_type, &src_oper_type))
                {
                    return FALSE;
                }

                dest_oper_type = VAL_fileCopyDestOperType_publickey;
                break;
        } /* End of switch */

        XFER_MGR_SetFileCopySrcOperType(src_oper_type);
        XFER_MGR_SetFileCopyDestOperType(dest_oper_type);
        XFER_MGR_SetFileCopySrcFileName(srcfile);
        XFER_MGR_SetFileCopyDestFileName(destfile);
        file_copy_mgt_entry.file_type = file_type;

        XFER_MGR_SetFileCopyServerUserName(username);
        XFER_MGR_SetFileCopyServerPassword(password);

        memcpy(&(file_copy_mgt_entry.server_address), server_ip_p, sizeof(file_copy_mgt_entry.server_address));
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }

    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    mtext->src_oper_type = src_oper_type;
    mtext->dest_oper_type = dest_oper_type;

    msg.mtype = XFER_MGR_COPYFILE;
    msg.mtext = mtext;

    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_type = 1;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

} /*  End of XFER_MGR_CopyFile */

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
    UI32_T stream_max_length)
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    UI32_T  src_oper_type;
    BOOL_T  set_is_busy_flag = FALSE;

    /* BODY
     */
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
    	BUFFER_MGR_Free(x_buf);
        return FALSE;
    }

    if (FALSE == XFER_MGR_ServerTypeToSrcOperType(server_type, &src_oper_type))
    {
        return FALSE;
    }

    if (FALSE == XFER_MGR_IsValidServerIP(server_ip_p))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_REMOTETOSTREAM_FUNC_NO,
                                EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
		BUFFER_MGR_Free(x_buf);
        return FALSE;
    }

    if (srcfile == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_REMOTETOSTREAM_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,srcfile);
		BUFFER_MGR_Free(x_buf);
        return FALSE;
    }

    if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_REMOTETOSTREAM_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,srcfile);
		BUFFER_MGR_Free(x_buf);
        return FALSE;
    }

    if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_REMOTETOSTREAM_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
		BUFFER_MGR_Free(x_buf);
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_REMOTETOSTREAM)))== NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_REMOTETOSTREAM_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
		BUFFER_MGR_Free(x_buf);
        return FALSE;
    }

    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    mtext->callback =      callback;
    mtext->cookie   =      cookie;
    memcpy(&(mtext->server_ip), server_ip_p, sizeof(mtext->server_ip));
    mtext->buf =           x_buf;
    mtext->file_type =     file_type;
    mtext->is_next_boot =  info.is_next_boot;
    mtext->length =        stream_max_length;
    mtext->ipc_message_q = ipc_message_q;
    strcpy ((char *)mtext->srcfile, (char *)srcfile);
    mtext->server_type = server_type;
    mtext->src_oper_type = src_oper_type;
    mtext->dest_oper_type = VAL_fileCopyDestOperType_runningCfg;

    XFER_MGR_CopyUserInfoByServerType(server_type, username, password, mtext->username, mtext->password);

    msg.mtype = XFER_MGR_REMOTETOSTREAM;
    msg.mtext =  mtext;

    if (FALSE == is_file_copy_mgt_used)
    {
        XFER_MGR_SetFileCopySrcOperType(src_oper_type);

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
        if (TRUE == info.is_partial_provision)
        {
            XFER_MGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_addRunningCfg);
        }
        else
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */
        {
            XFER_MGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_runningCfg);
        }

        XFER_MGR_SetFileCopySrcFileName(srcfile);
        file_copy_mgt_entry.file_type = file_type;
        memcpy(&(file_copy_mgt_entry.server_address), server_ip_p, sizeof(file_copy_mgt_entry.server_address));
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));
    req_msg_p->msg_type =1;
    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free(mtext);
		BUFFER_MGR_Free(x_buf);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

} /*  End of XFER_MGR_RemoteToStream */

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
                                void (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if(desfile == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,desfile);
        return FALSE;
    }

    if (strlen((char *)desfile) > XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,desfile);
        return FALSE;
    }

    if(FALSE == XFER_MGR_IsValidServerIP(server_p))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
                                EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
        return FALSE;
    }
    return TRUE;
}

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
    void (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    UI32_T  dest_oper_type;

    if (FALSE == XFER_MGR_ServerTypeToDestOperType(server_type, &dest_oper_type))
    {
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_STREAMTOREMOTE)) ) == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    //ext->length =        strlen((char *)mtext->buf);
    /*mtext->server_ip =     server_ip;*/
    mtext->file_type =     file_type;
    mtext->is_next_boot =  info.is_next_boot;
    mtext->callback =      callback;
    mtext->cookie   =      cookie;
    mtext->ipc_message_q = ipc_message_q;
    mtext->length = buffer_length;
    mtext->offset = offset;
    mtext->server_type = server_type;
    mtext->src_oper_type = VAL_fileCopySrcOperType_runningCfg;
    mtext->dest_oper_type = dest_oper_type;

    XFER_MGR_CopyUserInfoByServerType(server_type, username, password, mtext->username, mtext->password);

    memcpy(&(mtext->server_ip), server_p, sizeof(mtext->server_ip));
    strcpy ((char *)mtext->destfile, (char *)desfile);

    msg.mtype = XFER_MGR_STREAMTOREMOTE;
    msg.mtext =  mtext;

    if (FALSE == is_file_copy_mgt_used)
    {
        XFER_MGR_SetFileCopySrcOperType(VAL_fileCopySrcOperType_runningCfg);
        XFER_MGR_SetFileCopyDestOperType(dest_oper_type);
        XFER_MGR_SetFileCopyDestFileName(desfile);
        file_copy_mgt_entry.file_type = file_type;
        /*file_copy_mgt_entry.tftp_server = server_ip;*/
        memcpy(&(file_copy_mgt_entry.server_address), server_p, sizeof(file_copy_mgt_entry.server_address));
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));
    req_msg_p->msg_type = 1;

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

} /*  End of XFER_MGR_RemoteToStream */

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
    void (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    BOOL_T  set_is_busy_flag = FALSE;
    UI32_T  dest_oper_type;

    if (FALSE == XFER_MGR_ServerTypeToDestOperType(server_type, &dest_oper_type))
    {
        return FALSE;
    }

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    if(desfile == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,desfile);
        return FALSE;
    }

    if (strlen((char *)desfile) > XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,desfile);
        return FALSE;
    }

    if(FALSE == XFER_MGR_IsValidServerIP(server_p))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
                                EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_STREAMTOREMOTE)) ) == NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if((mtext->buf=(UI8_T *)BUFFER_MGR_Allocate()) == NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free (mtext);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOREMOTE_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if(file_type == FS_FILE_TYPE_CONFIG && strcmp((char *)desfile, SYS_DFLT_restartConfigFile) == 0)
    {
        strcpy((char *)mtext->buf, (char *)sys_dfltcfg);
    }
    else if(file_type != FS_FILE_TYPE_CONFIG)
	{
	    XFER_MGR_Transaction_End(set_is_busy_flag);
		BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_STREAMTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
        return FALSE;
	}

    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    mtext->length =        strlen((char *)mtext->buf);
    /*mtext->server_ip =     server_ip;*/
    mtext->file_type =     file_type;
    mtext->is_next_boot =  info.is_next_boot;
    mtext->callback =      callback;
    mtext->cookie   =      cookie;
    mtext->ipc_message_q = ipc_message_q;
    mtext->server_type = server_type;
    mtext->src_oper_type = VAL_fileCopySrcOperType_runningCfg;
    mtext->dest_oper_type = dest_oper_type;

    XFER_MGR_CopyUserInfoByServerType(server_type, username, password, mtext->username, mtext->password);

    memcpy(&(mtext->server_ip), server_p, sizeof(mtext->server_ip));
    strcpy ((char *)mtext->destfile, (char *)desfile);

    msg.mtype = XFER_MGR_STREAMTOREMOTE;
    msg.mtext =  mtext;

    if (FALSE == is_file_copy_mgt_used)
    {
        XFER_MGR_SetFileCopySrcOperType(VAL_fileCopySrcOperType_runningCfg);
        XFER_MGR_SetFileCopyDestOperType(dest_oper_type);
        XFER_MGR_SetFileCopyDestFileName(desfile);
        file_copy_mgt_entry.file_type = file_type;
        /*file_copy_mgt_entry.tftp_server = server_ip;*/
        memcpy(&(file_copy_mgt_entry.server_address), server_p, sizeof(file_copy_mgt_entry.server_address));
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));
    req_msg_p->msg_type = 1;

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

} /*  End of XFER_MGR_RemoteToStream */

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
    void (*callback) (void *cookie, UI32_T status))
{

    XFER_Msg_T msg;
    Mtext_T *mtext;
    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    UI32_T ret;


    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_STREAMTOLOCAL)) ) == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

//    mtext->buf = buffer;
    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    strcpy ((char *)mtext->destfile, (char *)filename);
    mtext->file_type =       file_type;
    mtext->is_next_boot =    info.is_next_boot;
    mtext->cookie   =        cookie;
    mtext->callback =        callback;
//    mtext->length =          strlen((char *)mtext->buf);
    mtext->ipc_message_q =   ipc_message_q;
    mtext->length = buffer_length;
    mtext->offset = offset;
    mtext->src_oper_type = VAL_fileCopySrcOperType_runningCfg;
    mtext->dest_oper_type = VAL_fileCopyDestOperType_file;

    msg.mtype = XFER_MGR_STREAMTOLOCAL;
    msg.mtext = mtext;

    if (FALSE == is_file_copy_mgt_used)
    {
        XFER_MGR_SetFileCopySrcOperType(VAL_fileCopySrcOperType_runningCfg);
        XFER_MGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_file);
        XFER_MGR_SetFileCopyDestFileName(filename);
        file_copy_mgt_entry.file_type = file_type;
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));

   req_msg_p->cmd= SYS_MODULE_XFER;
      req_msg_p->msg_type = 1;

    ret = SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL);


    if(ret  != SYSFUN_OK)
    {
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;
}

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
    void (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    UI32_T ret;
    BOOL_T set_is_busy_flag = FALSE;


    /* BODY
     */
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    if (filename == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_STREAMTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,filename);
        return FALSE;
    }

    if (strlen((char *)filename) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_STREAMTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,filename);
        return FALSE;
    }

    if (FS_GenericFilenameCheck(filename, file_type) != FS_RETURN_OK)
    {
        return FALSE;
    }

    if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_STREAMTOLOCAL)) ) == NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if((mtext->buf=(UI8_T *)BUFFER_MGR_Allocate()) == NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free (mtext);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_STREAMTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if(file_type == FS_FILE_TYPE_CONFIG && strcmp((char *)filename, SYS_DFLT_restartConfigFile) == 0)
    {
        strcpy((char *)mtext->buf, (char *)sys_dfltcfg);
    }
	else if(file_type != FS_FILE_TYPE_CONFIG)
	{
	    XFER_MGR_Transaction_End(set_is_busy_flag);
		BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_STREAMTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
        return FALSE;
	}

    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    strcpy ((char *)mtext->destfile, (char *)filename);
    mtext->file_type =       file_type;
    mtext->is_next_boot =    info.is_next_boot;
    mtext->cookie   =        cookie;
    mtext->callback =        callback;
    mtext->length =          strlen((char *)mtext->buf);
    mtext->ipc_message_q =   ipc_message_q;
    mtext->src_oper_type = VAL_fileCopySrcOperType_runningCfg;
    mtext->dest_oper_type = VAL_fileCopyDestOperType_file;

    msg.mtype = XFER_MGR_STREAMTOLOCAL;
    msg.mtext = mtext;

    if (FALSE == is_file_copy_mgt_used)
    {
        XFER_MGR_SetFileCopySrcOperType(VAL_fileCopySrcOperType_runningCfg);
        XFER_MGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_file);
        XFER_MGR_SetFileCopyDestFileName(filename);
        file_copy_mgt_entry.file_type = file_type;
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));

    req_msg_p->cmd= SYS_MODULE_XFER;
    req_msg_p->msg_type = 1;

    ret = SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL);

    if(ret  != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

} /* End of XFER_MGR_StreamToLocal */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_LocalToStream
 *------------------------------------------------------------------------
 * FUNCTION: This function will from file system read file to buffer
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
    UI32_T stream_max_length)
{
    Mtext_T *mtext;
    UI32_T unit_id;
    BOOL_T ret = FALSE;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return -1;
    }

    STKTPLG_POM_GetMyUnitID(&unit_id);
    if (filename == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOSTREAM_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,filename);
        return FS_RETURN_ERROR;
    }

    if (strlen((char *)filename) > XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOSTREAM_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,filename);
        return FS_RETURN_ERROR;
    }

    if (FS_FilenameCheck(filename)!= FS_RETURN_OK)
    {
        return FS_RETURN_ERROR;
    }

    if (NULL == (mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER,
        XFER_TYPE_TRACE_ID_XFER_MGR_LOCALTOSTREAM))))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOSTREAM_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FS_RETURN_ERROR;
    }

    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    mtext->file_type = FS_FILE_TYPE_CONFIG;
    mtext->is_next_boot = info.is_next_boot;
    mtext->src_oper_type = VAL_fileCopySrcOperType_file;
    mtext->dest_oper_type = VAL_fileCopyDestOperType_runningCfg;
    strncpy((char *)mtext->srcfile, (char *)filename, sizeof(mtext->srcfile)-1);
    mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';

    if (FALSE == is_file_copy_mgt_used)
    {
        XFER_MGR_SetFileCopySrcOperType(VAL_fileCopySrcOperType_file);
        XFER_MGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_runningCfg);
        XFER_MGR_SetFileCopySrcFileName(filename);
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }

    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    ret = FS_ReadFile(unit_id, filename, xbuffer, stream_max_length, xbuf_length);

    if (ret == FS_RETURN_OK)
    {
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
        mtext->length=*xbuf_length;
    }
    else
    {
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
    }

    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    mtext->status = file_copy_mgt_entry.status;
    XFER_MGR_SendFileCopyTrap(mtext);
    is_file_copy_mgt_used = FALSE;
    L_MM_Free(mtext);
    return ret;
}/* End of XFER_MGR_LocalToStream() */

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
UI32_T  XFER_MGR_ReadSystemConfig(UI8_T *xbuffer, UI32_T *xbuf_length)
{
    UI8_T bootcfg_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1];
    UI32_T retval;
    UI32_T unit_id;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return -1;
    }

    STKTPLG_POM_GetMyUnitID(&unit_id);
    retval = FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, bootcfg_filename);

    if( retval != FS_RETURN_OK )
    {
        return retval;
    }


    return FS_ReadFile( unit_id, bootcfg_filename,  xbuffer, SYS_ADPT_MAX_FILE_SIZE, xbuf_length);
}/* End of XFER_MGR_ReadSystemConfig() */

/* erica 09/02/02 for stackable system */
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
    void (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    BOOL_T  set_is_busy_flag = FALSE;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    /* BODY
     */
    if (destfile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_UNITTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,destfile);
        return FALSE;
    }

    if (srcfile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_UNITTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,srcfile);
        return FALSE;
    }

    if (strlen((char *)destfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_UNITTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,destfile);
        return FALSE;
    }

    if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_UNITTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,srcfile);
        return FALSE;
    }

    if (FS_GenericFilenameCheck(destfile, file_type) != FS_RETURN_OK)
    {
        return FALSE;
    }

    /* Check unit is exist ?
     */
    {
        FS_File_Attr_T checked_file;
        strncpy((char *)checked_file.file_name, (char *)srcfile, sizeof(checked_file.file_name) - 1);
        checked_file.file_name[sizeof(checked_file.file_name)-1] = '\0';
        checked_file.file_type_mask = FS_FILE_TYPE_MASK(file_type);

        if (FS_GetFileInfo(unit_id,&checked_file) != FS_RETURN_OK)
        {
            return FALSE;
        }
    }

    if (FS_GenericFilenameCheck(srcfile, file_type) != FS_RETURN_OK)
    {
        return FALSE;
    }

    if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_UNITTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_UNITTOLOCAL)) )== NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_UNITTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if ( (mtext->buf = BUFFER_MGR_Allocate()) == 0 )
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free (mtext);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_UNITTOLOCAL_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    mtext->unit_id =        unit_id;
    mtext->cookie =         cookie;
    mtext->callback =       callback;
    mtext->file_type =      file_type;
    mtext->is_next_boot =   info.is_next_boot;
    mtext->ipc_message_q =  ipc_message_q;
    mtext->src_oper_type = VAL_fileCopySrcOperType_unit;
    mtext->dest_oper_type = VAL_fileCopyDestOperType_file;

    strcpy ((char *)mtext->destfile, (char *)destfile);
    strcpy ((char *)mtext->srcfile, (char *)srcfile);

    msg.mtype = XFER_MGR_COPYUNITFILE;
    msg.mtext = mtext;

    if (FALSE == is_file_copy_mgt_used)
    {
        XFER_MGR_SetFileCopySrcOperType(VAL_fileCopySrcOperType_unit);
        XFER_MGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_file);
        XFER_MGR_SetFileCopySrcFileName(srcfile);
        XFER_MGR_SetFileCopyDestFileName(destfile);
        XFER_MGR_SetFileCopyUnit(unit_id);
        file_copy_mgt_entry.file_type = file_type;
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_type = 1;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

}/* End of XFER_MGR_UnitToLocal() */

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
    void (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    BOOL_T  set_is_busy_flag = FALSE;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }
    /* BODY
     */
    if (destfile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,destfile);
        return FALSE;
    }

    if (srcfile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,srcfile);
        return FALSE;
    }

    if (strlen((char *)destfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,destfile);
        return FALSE;
    }

    if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,srcfile);
        return FALSE;
    }

    if (FS_GenericFilenameCheck(destfile, file_type) != FS_RETURN_OK)
    {
        return FALSE;
    }

    /* Check unit is exist ?
     */
    {
        FS_File_Attr_T checked_file;
        strncpy((char *)checked_file.file_name, (char *)srcfile, sizeof(checked_file.file_name) - 1);
        checked_file.file_name[sizeof(checked_file.file_name)-1] = '\0';
        checked_file.file_type_mask = FS_FILE_TYPE_MASK(file_type);

        if (FS_GetFileInfo(unit_id,&checked_file) != FS_RETURN_OK)
        {
            return FALSE;
        }
    }

    if (FS_GenericFilenameCheck(srcfile, file_type) != FS_RETURN_OK)
    {
        return FALSE;
    }

    if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_LOCALTOUNIT)) )== NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if ( (mtext->buf = BUFFER_MGR_Allocate()) == 0 )
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free (mtext);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    mtext->unit_id =        unit_id;
    mtext->cookie =         cookie;
    mtext->callback =       callback;
    mtext->file_type =      file_type;
    mtext->is_next_boot =   info.is_next_boot;
    mtext->ipc_message_q =  ipc_message_q;

    strcpy ((char *)mtext->destfile, (char *)destfile);
    strcpy ((char *)mtext->srcfile, (char *)srcfile);
    mtext->src_oper_type = VAL_fileCopySrcOperType_file;
    mtext->dest_oper_type = VAL_fileCopyDestOperType_unit;

    msg.mtype = XFER_MGR_COPYFILEUNIT;
    msg.mtext = mtext;

    if (FALSE == is_file_copy_mgt_used)
    {
        XFER_MGR_SetFileCopySrcOperType(VAL_fileCopySrcOperType_file);
        XFER_MGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_unit);
        XFER_MGR_SetFileCopySrcFileName(srcfile);
        XFER_MGR_SetFileCopyDestFileName(destfile);
        XFER_MGR_SetFileCopyUnit(unit_id);
        file_copy_mgt_entry.file_type = file_type;
        file_copy_mgt_entry.action = VAL_fileCopyAction_copy;
    }
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_type = 1;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

}/* End of XFER_MGR_LocalToUnit() */

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
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetTftpMgtEntry(XFER_MGR_TftpMgtEntry_T *tftp_mgt_entry_Info)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    memcpy(tftp_mgt_entry_Info, &TftpMgrEntry, sizeof(TftpMgrEntry));
    return TRUE;
} /* End of XFER_MGR_GetTftpMgtEntry() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpFileType
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    :ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_SetTftpFileType(UI32_T  tftp_file_type)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if(tftp_file_type == VAL_tftpFileType_opcode)
    {
        tftp_file_type = FS_TYPE_OPCODE_FILE_TYPE;
    }
    else if(tftp_file_type == VAL_tftpFileType_config)
    {
        tftp_file_type = FS_FILE_TYPE_CONFIG;
    }
    TftpMgrEntry.tftpFileType = tftp_file_type;
    return TRUE;
} /* End of XFER_MGR_SetTftpFileType() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpSrcFile
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_SetTftpSrcFile(UI8_T  *tftp_src_file)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if(tftp_src_file == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETTFTPSRCFILE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,tftp_src_file);
        return FALSE;
    }

    if (strlen((char *)tftp_src_file) > XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETTFTPSRCFILE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,tftp_src_file);
        return FALSE;
    }

    strcpy ((char *)TftpMgrEntry.tftpSrcFile, (char *)tftp_src_file);
    return TRUE;
}    /* End of XFER_MGR_SetTftpSrcFile() */


/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpDestFile
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   :
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_SetTftpDestFile(UI8_T  *tftp_des_file)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if(tftp_des_file== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETTFTPDESTFILE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,tftp_des_file);
        return FALSE;
    }

    if (strlen((char *)tftp_des_file) > XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETTFTPDESTFILE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,tftp_des_file);
        return FALSE;
    }

    strcpy ((char *)TftpMgrEntry.tftpDestFile, (char *)tftp_des_file);
    return TRUE;
}  /* End of XFER_MGR_SetTftpDestFile() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpServer
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the tftp info
 * INPUT   : L_INET_AddrIp_T  *tftp_server
 * OUTPUT  : tftp info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/tftpMgt
 *------------------------------------------------------------------------*/
UI32_T XFER_MGR_SetTftpServer(L_INET_AddrIp_T  *tftp_server)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if(FALSE == XFER_MGR_IsValidServerIP(tftp_server))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_SETTFTPSERVER_FUNC_NO,
                                EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    memcpy( &TftpMgrEntry.tftpServer, tftp_server, sizeof(TftpMgrEntry.tftpServer));
    return TRUE;
} /* End of XFER_MGR_SetTftpServer() */

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
    UI32_T tftp_active)
{
    UI32_T          unit_id;
    FS_File_Attr_T  file_attr;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if(TftpMgrEntry.tftpDestFile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETTFTPACTIVE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,"destfile");
        return FALSE;
    }

    if(TftpMgrEntry.tftpSrcFile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETTFTPACTIVE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO, "srcfile");
        return FALSE;
    }

    /* the check item move to xxx_syn
     * check the number of operation files in flash
     */

    if (info.is_busy)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_SETTFTPACTIVE_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    STKTPLG_POM_GetMyUnitID(&unit_id);

    TftpMgrEntry.tftpAction = tftp_active;
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(TftpMgrEntry.tftpFileType);

    switch ( tftp_active )
    {
        case VAL_tftpAction_downloadToPROM:
              if (FS_GenericFilenameCheck(TftpMgrEntry.tftpDestFile, TftpMgrEntry.tftpFileType) != FS_RETURN_OK)
              {
                 TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                 return FALSE;
              }

             if (!XFER_MGR_CopyFile(user_info_p,
                                    file_copy_mgt_entry.publickey_username,
                                    file_copy_mgt_entry.publickey,
                                    &TftpMgrEntry.tftpServer,
                                    TftpMgrEntry.tftpDestFile,
                                    TftpMgrEntry.tftpSrcFile,
                                    TftpMgrEntry.tftpFileType,
                                    XFER_MGR_REMOTE_TO_LOCAL,
                                    XFER_MGR_REMOTE_SERVER_TFTP,
                                    NULL, NULL,
                                    0,
                                    SYS_BLD_XFER_GROUP_IPCMSGQ_KEY,
                                    0))
             {
                 TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                 return FALSE;
             }
             return TRUE;

        case VAL_tftpAction_upload:
            /*check file exist*/
            if (FS_GenericFilenameCheck(TftpMgrEntry.tftpSrcFile, TftpMgrEntry.tftpFileType) != FS_RETURN_OK)
            {
                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                return FALSE;
            }
            strcpy((char *)file_attr.file_name, (char *)TftpMgrEntry.tftpSrcFile);
            if( FS_GetFileInfo(unit_id, &file_attr) != FS_RETURN_OK)
            {
                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                return FALSE;
            }

            if (!XFER_MGR_CopyFile(user_info_p,
                                   file_copy_mgt_entry.publickey_username,
                                   file_copy_mgt_entry.publickey,
                                   &TftpMgrEntry.tftpServer,
                                   TftpMgrEntry.tftpDestFile,
                                   TftpMgrEntry.tftpSrcFile,
                                   TftpMgrEntry.tftpFileType,
                                   XFER_MGR_LOCAL_TO_REMOTE,
                                   XFER_MGR_REMOTE_SERVER_TFTP,
                                   NULL, NULL,
                                   0,
                                   SYS_BLD_XFER_GROUP_IPCMSGQ_KEY,
                                   0))
            {
                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                return FALSE;
            }
            return TRUE;

        case VAL_tftpAction_notDownloading:
        case VAL_tftpAction_downloadToRAM:
             TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
             EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETTFTPACTIVE_FUNC_NO,
             EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"download");
             return FALSE;
        } /* End of switch */
   EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_SETTFTPACTIVE_FUNC_NO,
   EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
   return FALSE;
}/* End of XFER_MGR_SetTftpActive() */

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetSWVersionFromONIEInstallerFile
 *------------------------------------------------------------------------
 * FUNCTION: This function get the software version of the given ONIE
 *           installer file.
 * INPUT   : installer_file_full_path_p - full path to the ONIE installer file.
 * OUTPUT  : software_version_p - pointer of software version
 * RETURN  : TRUE - success
 *           FALSE - fail
 * NOTE    : 1. Only support ONIE installer file
 *           2. This function will not validate the input and output argument
 *              because the caller shall have done this.
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_GetSWVersionFromONIEInstallerFile(const char* installer_file_full_path_p, UI8_T *software_version_p)
{
    FILE *fp=NULL;
    char *shell_cmd_buf_p=NULL;
    int shell_cmd_buf_size, pos;
    BOOL_T ret_val=TRUE;

    shell_cmd_buf_size=(sizeof(AOS_ONIE_GET_INSTALLER_VER_SCRIPT_FILENAME)-1) +
        strlen(installer_file_full_path_p) +
        SYS_ADPT_FW_VER_STR_LEN +
        1 + /* for one space */
        1; /* for terminating null char */
    shell_cmd_buf_p=L_MM_Malloc(shell_cmd_buf_size, L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_GETSWVERSIONFROMONIEINSTALLERFILE));

    if (shell_cmd_buf_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d):L_MM_Malloc %d bytes failed.\r\n", __FUNCTION__, __LINE__,
            shell_cmd_buf_size);
        return FALSE;
    }

    snprintf(shell_cmd_buf_p, shell_cmd_buf_size, "%s %s",
        AOS_ONIE_GET_INSTALLER_VER_SCRIPT_FILENAME, installer_file_full_path_p);
    if (onie_installer_debug_flag==TRUE)
    {
        BACKDOOR_MGR_Printf("%s:shell_cmd_buf_p='%s'\r\n", __FUNCTION__,
            shell_cmd_buf_p);
    }

    fp = popen(shell_cmd_buf_p, "r");
    if (fp == NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to run %s\r\n",
            __FUNCTION__, __LINE__, AOS_ONIE_GET_INSTALLER_VER_SCRIPT_FILENAME);
        ret_val=FALSE;
        goto error_exit;
    }
    else
    {
        if (fgets((char*)(software_version_p), SYS_ADPT_FW_VER_STR_LEN, fp)==NULL)
        {
            ret_val=FALSE;
            BACKDOOR_MGR_Printf("%s(%d):Failed to get version from script %s\r\n",
                __FUNCTION__, __LINE__, AOS_ONIE_GET_INSTALLER_VER_SCRIPT_FILENAME);
        }
        else
        {
            if (strncmp(SCRIPT_ERROR_STR, (char*)software_version_p, sizeof(SCRIPT_ERROR_STR)-1)==0)
            {
                ret_val=FALSE;
            }
            else
            {
                software_version_p[SYS_ADPT_FW_VER_STR_LEN]='\0';
                /* trim the non-printable trailing char
                 */
                L_CHARSET_TrimTrailingNonPrintableChar((char*)software_version_p);
            }
        }
    }

error_exit:
    if(fp)
        pclose(fp);

    if(shell_cmd_buf_p)
        L_MM_Free(shell_cmd_buf_p);

    return ret_val;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetSWVersionFromONIEInstallerStream
 *------------------------------------------------------------------------
 * FUNCTION: This function get the software version of the given ONIE
 *           installer memory stream.
 * INPUT   : installer_stream_p - pointer to ONIE installer memory stream.
 *                                The length of the memory stream must be
 *                                AOS_ONIE_INSTALLER_MAX_IMG_HEADER_SIZE.
 * OUTPUT  : software_version_p - pointer of software version
 * RETURN  : TRUE - success
 *           FALSE - fail
 * NOTE    : 1. Only support ONIE installer data
 *           2. This function will not validate the input and output argument
 *              because the caller shall have done this.
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_GetSWVersionFromONIEInstallerStream(const UI8_T* installer_stream_p, UI8_T *software_version_p)
{
    char* tmpfilename_p;
    UI32_T tmpfilename_size;

    if (FS_GetRequiredTmpFilenameBufSize(XFER_MGR_CSC_NAME, &tmpfilename_size)!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_GetRequiredTmpFilenameBufSize returns error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    tmpfilename_p=L_MM_Malloc(tmpfilename_size, L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_GETSWVERSIONFROMONIEINSTALLERDATA));

    if (tmpfilename_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)L_MM_Malloc failed for %lu bytes.\r\n", __FUNCTION__, __LINE__, (unsigned long)tmpfilename_size);
        return FALSE;
    }

    if (FS_CreateTmpFile(XFER_MGR_CSC_NAME, installer_stream_p, AOS_ONIE_INSTALLER_MAX_IMG_HEADER_SIZE,
        tmpfilename_size, tmpfilename_p)!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_CreateTmpFile error\r\n", __FUNCTION__, __LINE__);
        L_MM_Free(tmpfilename_p);
        return FALSE;
    }

    if (XFER_MGR_GetSWVersionFromONIEInstallerFile(tmpfilename_p, software_version_p)==FALSE)
    {
        if(FS_RemoveTmpFile(tmpfilename_p)!=FS_RETURN_OK)
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to remove temp file '%s'\r\n", __FUNCTION__,
                __LINE__, tmpfilename_p);
        }
        L_MM_Free(tmpfilename_p);
        return FALSE;
    }

    if(FS_RemoveTmpFile(tmpfilename_p)!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to remove temp file '%s'\r\n", __FUNCTION__,
            __LINE__, tmpfilename_p);
    }
    L_MM_Free(tmpfilename_p);
    return TRUE;
}

#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetSWVersion
 *------------------------------------------------------------------------
 * FUNCTION: This function will get local software version by file name
 * INPUT   : file_name_p - pointer of file name
 * OUTPUT  : software_version_p - pointer of software version
 * RETURN  : TRUE - success
 *           FALSE - fail
 * NOTE    : Only support runtime file(For SYS_CPNT_ONIE_SUPPORT==FALSE)
 *           Only support ONIE installer file(For SYS_CPNT_ONIE_SUPPORT==TRUE)
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetSWVersion(UI8_T *file_name_p, UI8_T *software_version_p)
{
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    char* full_path_file_name_buf_p;
    int   full_path_file_name_buf_size;
    const char* fs_opcode_dir_p=FS_GetOpCodeDirectory();
    BOOL_T ret_val;

    full_path_file_name_buf_size=strlen((char*)file_name_p) +
        strlen(fs_opcode_dir_p) + 1/* for terminating null char*/;

    full_path_file_name_buf_p=L_MM_Malloc(full_path_file_name_buf_size, L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_GETSWVERSION));
    if(full_path_file_name_buf_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)L_MM_Malloc failed for %d bytes.\r\n", __FUNCTION__,
            __LINE__, full_path_file_name_buf_size);
        return FALSE;
    }
    snprintf(full_path_file_name_buf_p, full_path_file_name_buf_size, "%s%s",
        fs_opcode_dir_p, file_name_p);
    ret_val=XFER_MGR_GetSWVersionFromONIEInstallerFile(full_path_file_name_buf_p,
        software_version_p);
    L_MM_Free(full_path_file_name_buf_p);
    return ret_val;
#else /* #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */
    UI32_T          unit_id;
    UI32_T          ret_state;
    FS_File_Attr_T  file_attr;
    image_header_t  imghdr;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    STKTPLG_POM_GetMyUnitID(&unit_id);

    memset((char*)&imghdr, 0, sizeof(imghdr));

    memset(&file_attr, 0, sizeof(file_attr));
    strcpy((char *)file_attr.file_name, (char *)file_name_p);
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_TYPE_OPCODE_FILE_TYPE);
    if( FS_RETURN_OK != FS_GetFileInfo(unit_id, &file_attr))
    {
        return FALSE;
    }

    ret_state  = FS_CopyFileContent( unit_id, file_name_p, 0,  (UI8_T *)&imghdr, sizeof(image_header_t));

    if(FS_RETURN_OK != ret_state)
    {
        return FALSE;
    }

    if(imghdr.ih_magic != IH_MAGIC)
        return FALSE;
    memcpy(software_version_p, imghdr.ih_ver, SYS_ADPT_FW_VER_STR_LEN+1);

    return TRUE;
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */
}/* End of XFER_MGR_GetSWVersion() */

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
BOOL_T XFER_MGR_GetTftpErrorMsg(UI8_T *tftp_error_msg)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    memcpy(tftp_error_msg, file_copy_mgt_entry.tftp_error_msg, strlen((char *)file_copy_mgt_entry.tftp_error_msg));
    tftp_error_msg[strlen((char *)file_copy_mgt_entry.tftp_error_msg)] = '\0';
    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetFileCopyMgtEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the file copy mgt info
 * INPUT   : None
 * OUTPUT  : file_copy_mgt
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetFileCopyMgtEntry(XFER_MGR_FileCopyMgt_T *file_copy_mgt)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    memcpy(file_copy_mgt, &file_copy_mgt_entry, sizeof(XFER_MGR_FileCopyMgt_T));
    return TRUE;
}

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
BOOL_T XFER_MGR_SetFileCopySrcOperType(UI32_T  src_oper_type)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    switch (src_oper_type)
    {
        case VAL_fileCopySrcOperType_startUpCfg:
        case VAL_fileCopySrcOperType_runningCfg:
        case VAL_fileCopySrcOperType_file:
        case VAL_fileCopySrcOperType_tftp:
        case VAL_fileCopySrcOperType_unit:
        case VAL_fileCopySrcOperType_http:
        case VAL_fileCopySrcOperType_ftp:
        case VAL_fileCopySrcOperType_sftp:
        case VAL_fileCopySrcOperType_ftps:
        case VAL_fileCopySrcOperType_usb:
            file_copy_mgt_entry.src_oper_type = src_oper_type;
            break;
        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYSRCOPERTYPE_FUNC_NO,
            EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
            return FALSE;
    }

    return TRUE;
}/* End of XFER_MGR_SetFileCopySrcOperType() */

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
BOOL_T XFER_MGR_SetFileCopySrcFileName(UI8_T  *src_file_name_p)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if(NULL == src_file_name_p)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYSRCFILENAME_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO, src_file_name_p);
        return FALSE;
    }

    if (strlen((char *)src_file_name_p) > XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYSRCFILENAME_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO, src_file_name_p);
        return FALSE;
    }

    strcpy ((char *)file_copy_mgt_entry.src_file_name, (char *)src_file_name_p);

    return TRUE;
}/* End of XFER_MGR_SetFileCopySrcFileName() */

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
BOOL_T XFER_MGR_SetFileCopyDestOperType(UI32_T  dest_oper_type)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    switch (dest_oper_type)
    {
        case VAL_fileCopyDestOperType_startUpCfg:
        case VAL_fileCopyDestOperType_runningCfg:
#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
        case VAL_fileCopyDestOperType_addRunningCfg:
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */
        case VAL_fileCopyDestOperType_file:
        case VAL_fileCopyDestOperType_tftp:
        case VAL_fileCopyDestOperType_unit:
        case VAL_fileCopyDestOperType_publickey:
        case VAL_fileCopyDestOperType_ftp:
        case VAL_fileCopyDestOperType_sftp:
        case VAL_fileCopyDestOperType_ftps:
#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
        case VAL_fileCopyDestOperType_oamRemote:
#endif
#endif
#endif
#endif
            file_copy_mgt_entry.dest_oper_type = dest_oper_type;
            break;
        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYDESTOPERTYPE_FUNC_NO,
            EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
            return FALSE;
    }

    return TRUE;
}/* End of XFER_MGR_SetFileCopyDestOperType() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyDestFileName
 *------------------------------------------------------------------------
 * FUNCTION: Set the destination file name
 * INPUT   : UI8_T  *dest_file_name_p   -- the starting address of destination file name
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : if the destination operation type is VAL_fileCopyDestOperType_runningCfg(2)
 *           or VAL_fileCopyDestOperType_startUpCfg(3) or
 *           VAL_fileCopyDestOperType_addRunningCfg (15),this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopyDestFileName(UI8_T  *dest_file_name_p)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if(NULL == dest_file_name_p)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYDESTFILENAME_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO, dest_file_name_p);
        return FALSE;
    }

    if (strlen((char *)dest_file_name_p) > XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYDESTFILENAME_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO, dest_file_name_p);
        return FALSE;
    }

    strcpy ((char *)file_copy_mgt_entry.dest_file_name, (char *)dest_file_name_p);

    return TRUE;
}/* End of XFER_MGR_SetFileCopyDestFileName() */

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
BOOL_T XFER_MGR_SetFileCopyServerInetAddress(L_INET_AddrIp_T *server_p)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    if (FALSE == XFER_MGR_IsValidServerIP(server_p))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYSERVERINETADDRESS_FUNC_NO,
                                EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    memcpy(&file_copy_mgt_entry.server_address, server_p, sizeof(file_copy_mgt_entry.server_address));

    return TRUE;
}

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
BOOL_T XFER_MGR_SetFileCopyServerUserName(UI8_T *username)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    if (NULL == username)
    {
        return FALSE;
    }

    memset(file_copy_mgt_entry.username, 0, sizeof(file_copy_mgt_entry.username));
    strncpy((char *)file_copy_mgt_entry.username, (char *)username, MAXSIZE_fileCopyServerUserName);

    return TRUE;
}

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
BOOL_T XFER_MGR_SetFileCopyServerPassword(UI8_T *password)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    if (NULL == password)
    {
        return FALSE;
    }

    memset(file_copy_mgt_entry.password, 0, sizeof(file_copy_mgt_entry.password));
    strncpy((char *)file_copy_mgt_entry.password, (char *)password, MAXSIZE_fileCopyServerPassword);

    return TRUE;
}

#if (SYS_CPNT_SFTP == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetSftpHostkeyStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set the sftp host key status
 * INPUT   : UI32_T  hostkey_status  -- the hostkey status value
 *                                 VAL_fileCopySftpHostkeyStatus_Allow(1)
 *                                 VAL_fileCopySftpHostkeyStatus_Deny(2)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetSftpHostkeyStatus(UI32_T  hostkey_status)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    switch (hostkey_status)
    {
        case VAL_fileCopySftpHostkeyStatus_Allow:
        case VAL_fileCopySftpHostkeyStatus_Deny:
            file_copy_mgt_entry.host_key_status = hostkey_status;
            break;
        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYSRCOPERTYPE_FUNC_NO,
            EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"status");
            return FALSE;
    }
    return TRUE;
}
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

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
BOOL_T XFER_MGR_SetFileCopyFileType(UI32_T  file_type)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    switch (file_type)
    {
        case VAL_fileCopyFileType_opcode:
            file_copy_mgt_entry.file_type = FS_TYPE_OPCODE_FILE_TYPE;
            break;
        case VAL_fileCopyFileType_config:
            file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
            break;
        case VAL_fileCopyFileType_bootRom:
            file_copy_mgt_entry.file_type = FS_FILE_TYPE_DIAG;
            break;
        case VAL_fileCopyFileType_publickey:
            // FIXME: file_type of leaf value is user view
            //        file_copy_mgt_entry.file_type is programming view
            //        Not exist a correct corresponding relation between values.
            //
            //        It cause an issue that user copy public key and then show
            //        the result from log.
            //        The log will show previous action as 'public' wrong.
            //
            //        This time just patch when call to syslog for this cause.
            //        Ref [PATCH: wrong syslog for copying cert](AOS5600-52X-00408)
            //
            file_copy_mgt_entry.file_type = FS_FILE_TYPE_CERTIFICATE;
            break;
#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
#ifdef SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE
#if (SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE == SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE)
        case VAL_fileCopyFileType_oamRemoteFirmware:
            file_copy_mgt_entry.file_type = FS_FILE_TYPE_CPEFIRMWARE;
#endif
#endif
#endif
#endif

#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
        case VAL_fileCopyFileType_loader:
            file_copy_mgt_entry.file_type = FS_FILE_TYPE_TOTAL;
            break;
#endif /* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */

        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYFILETYPE_FUNC_NO,
            EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
            return FALSE;
    }

    return TRUE;
}/* End of XFER_MGR_SetFileCopyFileType() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyTftpServer
 *------------------------------------------------------------------------
 * FUNCTION: Set the IP address of the TFTP server
 * INPUT   : L_INET_AddrIp_T  *tftp_server -- the TFTP server IP address
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If neither source operation type nor destination operation type
 *           is tftp(4), this function could be ignored.
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopyTftpServer(L_INET_AddrIp_T  *tftp_server)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {

       return FALSE;
    }

    /* Check TFTP server IP */
    if (FALSE == XFER_MGR_IsValidServerIP(tftp_server))
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYTFTPSERVER_FUNC_NO,
                                EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    memcpy( &file_copy_mgt_entry.server_address, tftp_server, sizeof(file_copy_mgt_entry.server_address));

    return TRUE;
}/* End of XFER_MGR_SetFileCopyTftpServer() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetFileCopyUnit
 *------------------------------------------------------------------------
 * FUNCTION: Set the unit id
 * INPUT   : UI32_T  unit    -- the unit id
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_SetFileCopyUnit(UI32_T  unit)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if (!STKTPLG_POM_UnitExist(unit))
    {
        EH_MGR_Handle_Exception1 (SYS_MODULE_XFER,
                                  XFER_MGR_SETFILECOPYUNIT_FUNC_NO,
                                  EH_TYPE_MSG_NOT_EXIST,
                                  SYSLOG_LEVEL_INFO,
                                  "Unit");
        return FALSE;
    }

    file_copy_mgt_entry.unit = unit;

    return TRUE;
}/* End of XFER_MGR_SetFileCopyUnit() */

BOOL_T XFER_MGR_SetFileCopyPulicKeyType(UI32_T key_type)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    switch (key_type)
    {
        case VAL_fileCopyFilePublickeyType_rsa:
            file_copy_mgt_entry.publickey= KEY_TYPE_RSA;
            break;
        case VAL_fileCopyFilePublickeyType_dsa:
            file_copy_mgt_entry.publickey= KEY_TYPE_DSA;
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

BOOL_T XFER_MGR_SetFileCopyUsername(char  *user_name)
{
    if(NULL == user_name)
        return FALSE;

    if (strlen(user_name) > SYS_ADPT_MAX_USER_NAME_LEN || *user_name =='\x0')
    {
        return FALSE;
    }

    strcpy (file_copy_mgt_entry.publickey_username, user_name);

    return TRUE;
}/* End of XFER_MGR_SetFileCopyUsername() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRemotePublickey
 *------------------------------------------------------------------------
 * FUNCTION: Copy TFTP publickey
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
 *           cookie         -- the cookie of CLI working area; only CLI
 *                             need to pass in this argument.
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
static BOOL_T
XFER_MGR_CopyRemotePublickey(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    void (*callback) (void *cookie, UI32_T status))
{
    UI8_T *buffer = NULL;

    if((buffer=(UI8_T *)BUFFER_MGR_Allocate()) == NULL)
    {
	   return FALSE;
    }

    if (!XFER_MGR_RemoteToStream(user_info_p,
                                 server_type,
                                 &file_copy_mgt_entry.server_address,
                                 NULL, NULL,
                                 file_copy_mgt_entry.src_file_name,
                                 FS_FILE_TYPE_CERTIFICATE,
                                 buffer, (UI32_T)0, 0, (void *)0,
                                 USER_DSA_PUBLIC_KEY_FILE_LENGTH))
        return FALSE;
#if 0
    if (SSHD_PMGR_GetUserPublicKeyFromXfer(file_copy_mgt_entry.tftp_server,
                                          file_copy_mgt_entry.src_file_name, 0, 0 )!= TRUE)
    {
        return FALSE;
    }


    if (  SSHD_PMGR_SetUserPublicKey((UI8_T *)file_copy_mgt_entry.publickey_username, file_copy_mgt_entry.publickey)!= TRUE)
    	return FALSE;

    SSHD_PMGR_FreeBuffer();
#endif
#if 0
    /*for fix the bug : ES4827G-FLF-20-1319,  David Dai , 2007/7/5,*/
      /* Bug Desp: snmp reponse time out (2s) */
     AMS_SYSFUN_Sleep(100);/* previous value: 200 ticks */
   if (AMS_SSHD_MGR_SetUserPublicKey(file_copy_mgt_entry.publickey_username, (UI16_T)file_copy_mgt_entry.vr_id,file_copy_mgt_entry.publickey)!= RC_SSHD_OK)
    {

        return RC_RET_VALUE(RC_XFER_ERROR);
    }
#endif
    return TRUE;

}

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
    void (*callback) (void *cookie, UI32_T status))
{
    BOOL_T  is_successfully_copy;
    UI32_T unit_id;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    if (VAL_fileCopyAction_copy != action && VAL_fileCopyAction_abortTftp != action)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
        EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
        return FALSE;
    }

    if (info.is_busy)
    {
        if (VAL_fileCopyAction_abortTftp == action)
        {
            if(!XFER_MGR_AbortTftp())
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
                EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
                return FALSE;
            }
            return TRUE;
        }
        else
        {
            EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
            EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
            return FALSE;
        }
    }

#if (SYS_CPNT_SFTP == TRUE)
    if (   (VAL_fileCopySrcOperType_sftp == file_copy_mgt_entry.src_oper_type)
        || (VAL_fileCopyDestOperType_sftp == file_copy_mgt_entry.dest_oper_type))
    {
        ssh_set_hostkey_check(file_copy_mgt_entry.host_key_status == VAL_fileCopySftpHostkeyStatus_Allow);
    }
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

    file_copy_mgt_entry.action = action;
    is_successfully_copy = FALSE;
    is_file_copy_mgt_used = TRUE;
    switch (file_copy_mgt_entry.src_oper_type)
    {
        case VAL_fileCopySrcOperType_file:
            switch (file_copy_mgt_entry.dest_oper_type)
            {
                case VAL_fileCopyDestOperType_file:
                    is_successfully_copy = XFER_MGR_CopyFileFile(user_info_p, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_runningCfg:
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    is_successfully_copy = XFER_MGR_CopyFileRunning(user_info_p, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_startUpCfg:
                    STKTPLG_POM_GetMyUnitID(&unit_id);
                    if(NULL == file_copy_mgt_entry.dest_file_name ||
                       strcmp((char *)file_copy_mgt_entry.dest_file_name, "") == 0)
                    {
                        if (FS_RETURN_OK != FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, file_copy_mgt_entry.dest_file_name))
                        {
                            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                            is_file_copy_mgt_used = FALSE;
                            return FALSE;
                        }
                    }
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    info.is_next_boot = TRUE;
                    is_successfully_copy = XFER_MGR_CopyFileFile(user_info_p, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_tftp:
                    is_successfully_copy = XFER_MGR_CopyFileRemote(user_info_p, XFER_MGR_REMOTE_SERVER_TFTP, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_ftp:
                    is_successfully_copy = XFER_MGR_CopyFileRemote(user_info_p, XFER_MGR_REMOTE_SERVER_FTP, cookie, ipc_message_q, callback);
                    break;

#if (SYS_CPNT_SFTP == TRUE)
                case VAL_fileCopyDestOperType_sftp:
                    is_successfully_copy = XFER_MGR_CopyFileRemote(user_info_p, XFER_MGR_REMOTE_SERVER_SFTP, cookie, ipc_message_q, callback);
                    break;
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

                case VAL_fileCopyDestOperType_ftps:
                    is_successfully_copy = XFER_MGR_CopyFileRemote(user_info_p, XFER_MGR_REMOTE_SERVER_FTPS, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_unit:
                    is_successfully_copy = XFER_MGR_CopyFileUnit(user_info_p, cookie, ipc_message_q, callback);
                    break;

                default:
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    is_file_copy_mgt_used = FALSE;
                    EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
                    EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
                    return FALSE;
            }
            break;

        case VAL_fileCopySrcOperType_runningCfg:
            file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
            switch (file_copy_mgt_entry.dest_oper_type)
            {
                case VAL_fileCopyDestOperType_file:
                    is_successfully_copy = XFER_MGR_CopyRunningFile(user_info_p, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_startUpCfg:
                    STKTPLG_POM_GetMyUnitID(&unit_id);
                    if(NULL == file_copy_mgt_entry.dest_file_name ||
                       strcmp((char *)file_copy_mgt_entry.dest_file_name, "") == 0)
                    {
                        if (FS_RETURN_OK != FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, file_copy_mgt_entry.dest_file_name))
                        {
                            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                            is_file_copy_mgt_used = FALSE;
                            return FALSE;
                        }
                    }
                    info.is_next_boot = TRUE;
                    is_successfully_copy = XFER_MGR_CopyRunningFile(user_info_p, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_tftp:
                    is_successfully_copy = XFER_MGR_CopyRunningRemote(user_info_p, XFER_MGR_REMOTE_SERVER_TFTP, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_ftp:
                    is_successfully_copy = XFER_MGR_CopyRunningRemote(user_info_p, XFER_MGR_REMOTE_SERVER_FTP, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_ftps:
                    is_successfully_copy = XFER_MGR_CopyRunningRemote(user_info_p, XFER_MGR_REMOTE_SERVER_FTPS, cookie, ipc_message_q, callback);
                    break;

                default:
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    is_file_copy_mgt_used = FALSE;
                    EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
                    EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
                    return FALSE;
            }
            break;

        case VAL_fileCopySrcOperType_startUpCfg:
            STKTPLG_POM_GetMyUnitID(&unit_id);
            if (FS_RETURN_OK != FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, file_copy_mgt_entry.src_file_name))
            {
                file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                is_file_copy_mgt_used = FALSE;
                return FALSE;
            }
            file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
            switch (file_copy_mgt_entry.dest_oper_type)
            {
                case VAL_fileCopyDestOperType_file:
                    is_successfully_copy = XFER_MGR_CopyFileFile(user_info_p, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_runningCfg:
                    is_successfully_copy = XFER_MGR_CopyFileRunning(user_info_p, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_tftp:
                    is_successfully_copy = XFER_MGR_CopyFileRemote(user_info_p, XFER_MGR_REMOTE_SERVER_TFTP, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_ftp:
                    is_successfully_copy = XFER_MGR_CopyFileRemote(user_info_p, XFER_MGR_REMOTE_SERVER_FTP, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_ftps:
                    is_successfully_copy = XFER_MGR_CopyFileRemote(user_info_p, XFER_MGR_REMOTE_SERVER_FTPS, cookie, ipc_message_q, callback);
                    break;

                default:
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    is_file_copy_mgt_used = FALSE;
                    EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
                    EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
                    return FALSE;
            }
            break;

        case VAL_fileCopySrcOperType_tftp:
            switch (file_copy_mgt_entry.dest_oper_type)
            {
                case VAL_fileCopyDestOperType_file:
                    is_successfully_copy = XFER_MGR_CopyRemoteFile(user_info_p, XFER_MGR_REMOTE_SERVER_TFTP, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_runningCfg:
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    is_successfully_copy = XFER_MGR_CopyRemoteRunning(user_info_p, XFER_MGR_REMOTE_SERVER_TFTP, cookie, ipc_message_q, callback);
                    break;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
                case VAL_fileCopyDestOperType_addRunningCfg:
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    is_successfully_copy = XFER_MGR_AddRemoteRunning(user_info_p, XFER_MGR_REMOTE_SERVER_TFTP, cookie, ipc_message_q, callback);
                    break;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

                case VAL_fileCopyDestOperType_startUpCfg:
                    STKTPLG_POM_GetMyUnitID(&unit_id);
                    if(NULL == file_copy_mgt_entry.dest_file_name ||
                       strcmp((char *)file_copy_mgt_entry.dest_file_name, "") == 0)
                    {
                        if (FS_RETURN_OK != FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, file_copy_mgt_entry.dest_file_name))
                        {
                            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                            is_file_copy_mgt_used = FALSE;
                            return FALSE;
                        }
                    }
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    info.is_next_boot = TRUE;
                    is_successfully_copy = XFER_MGR_CopyRemoteFile(user_info_p, XFER_MGR_REMOTE_SERVER_TFTP, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_publickey:
                    is_successfully_copy = XFER_MGR_CopyRemotePublickey(user_info_p, XFER_MGR_REMOTE_SERVER_TFTP, cookie, callback);
                    break;

                default:
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    is_file_copy_mgt_used = FALSE;
                    EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
                    EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
                    return FALSE;
            }
            break;

        case VAL_fileCopySrcOperType_unit:
            switch (file_copy_mgt_entry.dest_oper_type)
            {
                case VAL_fileCopyDestOperType_file:
                    is_successfully_copy = XFER_MGR_CopyUnitFile(user_info_p, cookie, ipc_message_q, callback);
                    break;

                default:
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    is_file_copy_mgt_used = FALSE;
                    EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
                    EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
                    return FALSE;
            }
            break;

        case VAL_fileCopySrcOperType_ftp:
            switch (file_copy_mgt_entry.dest_oper_type)
            {
                case VAL_fileCopyDestOperType_file:
                    is_successfully_copy = XFER_MGR_CopyRemoteFile(user_info_p, XFER_MGR_REMOTE_SERVER_FTP, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_runningCfg:
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    is_successfully_copy = XFER_MGR_CopyRemoteRunning(user_info_p, XFER_MGR_REMOTE_SERVER_FTP, cookie, ipc_message_q, callback);
                    break;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
                case VAL_fileCopyDestOperType_addRunningCfg:
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    is_successfully_copy = XFER_MGR_AddRemoteRunning(user_info_p, XFER_MGR_REMOTE_SERVER_FTP, cookie, ipc_message_q, callback);
                    break;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

                case VAL_fileCopyDestOperType_startUpCfg:
                    STKTPLG_POM_GetMyUnitID(&unit_id);
                    if(NULL == file_copy_mgt_entry.dest_file_name ||
                       strcmp((char *)file_copy_mgt_entry.dest_file_name, "") == 0)
                    {
                        if (FS_RETURN_OK != FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, file_copy_mgt_entry.dest_file_name))
                        {
                            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                            is_file_copy_mgt_used = FALSE;
                            return FALSE;
                        }
                    }
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    info.is_next_boot = TRUE;
                    is_successfully_copy = XFER_MGR_CopyRemoteFile(user_info_p, XFER_MGR_REMOTE_SERVER_FTP, cookie, ipc_message_q, callback);
                    break;

                default:
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    is_file_copy_mgt_used = FALSE;
                    EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
                    EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
                    return FALSE;
            }
            break;

#if (SYS_CPNT_SFTP == TRUE)
        case VAL_fileCopySrcOperType_sftp:
            switch (file_copy_mgt_entry.dest_oper_type)
            {
                case VAL_fileCopyDestOperType_file:
                    is_successfully_copy = XFER_MGR_CopyRemoteFile(user_info_p, XFER_MGR_REMOTE_SERVER_SFTP, cookie, ipc_message_q, callback);
                    break;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
                case VAL_fileCopyDestOperType_addRunningCfg:
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    is_successfully_copy = XFER_MGR_AddRemoteRunning(user_info_p, XFER_MGR_REMOTE_SERVER_SFTP, cookie, ipc_message_q, callback);
                    break;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

                default:
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    is_file_copy_mgt_used = FALSE;
                    return FALSE;
            }
            break;
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

        case VAL_fileCopySrcOperType_ftps:
            switch (file_copy_mgt_entry.dest_oper_type)
            {
                case VAL_fileCopyDestOperType_file:
                    is_successfully_copy = XFER_MGR_CopyRemoteFile(user_info_p, XFER_MGR_REMOTE_SERVER_FTPS, cookie, ipc_message_q, callback);
                    break;

                case VAL_fileCopyDestOperType_runningCfg:
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    is_successfully_copy = XFER_MGR_CopyRemoteRunning(user_info_p, XFER_MGR_REMOTE_SERVER_FTPS, cookie, ipc_message_q, callback);
                    break;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
                case VAL_fileCopyDestOperType_addRunningCfg:
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    is_successfully_copy = XFER_MGR_AddRemoteRunning(user_info_p, XFER_MGR_REMOTE_SERVER_FTPS, cookie, ipc_message_q, callback);
                    break;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

                case VAL_fileCopyDestOperType_startUpCfg:
                    STKTPLG_POM_GetMyUnitID(&unit_id);
                    if(NULL == file_copy_mgt_entry.dest_file_name ||
                       strcmp((char *)file_copy_mgt_entry.dest_file_name, "") == 0)
                    {
                        if (FS_RETURN_OK != FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, file_copy_mgt_entry.dest_file_name))
                        {
                            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                            is_file_copy_mgt_used = FALSE;
                            return FALSE;
                        }
                    }
                    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
                    info.is_next_boot = TRUE;
                    is_successfully_copy = XFER_MGR_CopyRemoteFile(user_info_p, XFER_MGR_REMOTE_SERVER_FTPS, cookie, ipc_message_q, callback);
                    break;

                default:
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    is_file_copy_mgt_used = FALSE;
                    EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
                    EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
                    return FALSE;
            }
            break;

        default:
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            is_file_copy_mgt_used = FALSE;
            EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
            EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
            return FALSE;
    }

    if (FALSE == is_successfully_copy)
    {
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        is_file_copy_mgt_used = FALSE;
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
        EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO,"copy");
        return FALSE;
    }

    return TRUE;
}/* End of XFER_MGR_SetFileCopyAction() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_AbortTftp
 *------------------------------------------------------------------------
 * FUNCTION: abort what XFER is doing
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : only effect durring tftp is in progress
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_AbortTftp()
{
    BOOL_T  ret = FALSE;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if (TRUE == info.is_busy && XFER_DNLD_TFTP_SUCCESS != info.tftp_status)
    {
        file_copy_mgt_entry.action = VAL_fileCopyAction_abortTftp;
        XFER_DNLD_Abort();
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

#if (TRUE == SYS_CPNT_DBSYNC_TXT)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetDualStartupCfgFileName
 *------------------------------------------------------------------------
 * FUNCTION: Get 2 fixed startup config file names in turn
 * INPUT   : None
 * OUTPUT  : file_name
 * RETURN  : TRUE: succes;FALSE: if check error
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetDualStartupCfgFileName(UI8_T *file_name)
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    UI32_T  unit_id;
    int     file_num = 1;

    /* BODY
     */
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    STKTPLG_POM_GetMyUnitID(&unit_id);
    /* Use 2 fixed file names in turn
     */
    FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, file_name);

    if (strlen((char *)file_name) > 0)
    {
        if (0 == sscanf (file_name, SYS_DFLT_DBSYNC_TXT_DUAL_FILENAME, &file_num))
        {
            file_num = 1;
        }
        else
        {
             if( file_num == 1 || file_num == 2)
                file_num = 3 - file_num;
             else
                file_num = 1;
        }
    }
    sprintf((char *)file_name, SYS_DFLT_DBSYNC_TXT_DUAL_FILENAME, file_num);

    return  TRUE;
} /* End XFER_MGR_GetDualStartupCfgFileName() */

#endif /* #if (TRUE == SYS_CPNT_DBSYNC_TXT) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves TFTP retry times
 * INPUT   : None
 * OUTPUT  : retry_times_p  - TFTP retry times
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_GetTftpRetryTimes(UI32_T *retry_times_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (retry_times_p == NULL)
    {
        return FALSE;
    }

    *retry_times_p = XFER_OM_GetTftpRetryTimes();

    return  TRUE;
} /* End XFER_MGR_GetTftpRetryTimes() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetRunningTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves TFTP retry times
 * INPUT   : None
 * OUTPUT  : retry_times_p  - TFTP retry times
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE    : None
 *------------------------------------------------------------------------
 */
UI32_T XFER_MGR_GetRunningTftpRetryTimes(UI32_T *retry_times_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (retry_times_p == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *retry_times_p = XFER_OM_GetTftpRetryTimes();

    if (SYS_DFLT_TFTP_NUMBER_OF_RETRIES == *retry_times_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

} /* End XFER_MGR_GetTftpRetryTimes() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Set TFTP retry times
 * INPUT   : retry_times    - TFTP retry times
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_MGR_SetTftpRetryTimes(UI32_T retry_times)
{
    char    arg_buf[20] = {0};

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (    (retry_times > XFER_TYPE_MAX_TFTP_RETRY_TIMES)
        ||  (retry_times < XFER_TYPE_MIN_TFTP_RETRY_TIMES)
       )
    {
        sprintf(arg_buf, "TFTP Retry Times (%d-%d)",
        XFER_TYPE_MIN_TFTP_RETRY_TIMES, XFER_TYPE_MAX_TFTP_RETRY_TIMES );
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, 0, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return FALSE;
    }

    XFER_OM_SetTftpRetryTimes(retry_times);

    return  TRUE;
} /* End XFER_MGR_GetTftpRetryTimes() */

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
BOOL_T XFER_MGR_GetTftpTimeout(UI32_T *timeout_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (timeout_p == NULL)
    {
        return FALSE;
    }

    *timeout_p = XFER_OM_GetTftpTimeout();

    return  TRUE;
}

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
UI32_T XFER_MGR_GetRunningTftpTimeout(UI32_T *timeout_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (timeout_p == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *timeout_p = XFER_OM_GetTftpTimeout();

    if (XFER_TYPE_DFLT_TFTP_TIMEOUT == *timeout_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

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
BOOL_T XFER_MGR_SetTftpTimeout(UI32_T timeout)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (    (timeout > XFER_TYPE_MAX_TFTP_TIMEOUT)
        ||  (timeout < XFER_TYPE_MIN_TFTP_TIMEOUT)
       )
    {
        return FALSE;
    }

    XFER_OM_SetTftpTimeout(timeout);

    return  TRUE;
}

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - XFER_MGR_IsProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: Get provision complete status
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : provision complete status
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
static BOOL_T XFER_MGR_IsProvisionComplete()
{
    return is_provision_complete;
}

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
BOOL_T XFER_MGR_SetAutoOpCodeUpgradeStatus(UI32_T status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (    (VAL_fileAutoUpgradeOpCodeStatus_enabled != status)
        &&  (VAL_fileAutoUpgradeOpCodeStatus_disabled != status)
       )
    {
        return FALSE;
    }

    XFER_OM_SetAutoOpCodeUpgradeStatus(status);

    return TRUE;
} /* End XFER_MGR_SetAutoSwUpgradeStatus() */

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
BOOL_T XFER_MGR_GetAutoOpCodeUpgradeStatus(UI32_T *status_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (NULL == status_p)
    {
        return FALSE;
    }

    *status_p = XFER_OM_GetAutoOpCodeUpgradeStatus();

    return  TRUE;
} /* End XFER_MGR_GetAutoOpCodeUpgradeStatus() */

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
SYS_TYPE_Get_Running_Cfg_T XFER_MGR_GetRunningAutoOpCodeUpgradeStatus(UI32_T *status_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (NULL == status_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status_p = XFER_OM_GetAutoOpCodeUpgradeStatus();

    if (SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_STATUS == *status_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End XFER_MGR_GetRunningAutoOpCodeUpgradeStatus() */

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
BOOL_T XFER_MGR_SetAutoOpCodeUpgradeReloadStatus(UI32_T status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (    (VAL_fileAutoUpgradeOpCodeReloadStatus_enabled != status)
        &&  (VAL_fileAutoUpgradeOpCodeReloadStatus_disabled != status)
       )
    {
        return FALSE;
    }

    XFER_OM_SetAutoOpCodeUpgradeReloadStatus(status);

    return TRUE;
}

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
BOOL_T XFER_MGR_GetAutoOpCodeUpgradeReloadStatus(UI32_T *status_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (NULL == status_p)
    {
        return FALSE;
    }

    *status_p = XFER_OM_GetAutoOpCodeUpgradeReloadStatus();

    return  TRUE;
}

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
SYS_TYPE_Get_Running_Cfg_T XFER_MGR_GetRunningAutoOpCodeUpgradeReloadStatus(UI32_T *status_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (NULL == status_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status_p = XFER_OM_GetAutoOpCodeUpgradeReloadStatus();

    if (SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_RELOAD_STATUS == *status_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

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
BOOL_T XFER_MGR_SetAutoOpCodeUpgradePath(const char *path_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (NULL == path_p)
    {
        return FALSE;
    }

    /* If the path_p is default value, it dosn't check the validity.
     */
    if (0 != strcmp(SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_PATH, path_p))
    {
        if (FALSE == XFER_MGR_IsValidUrl(path_p))
        {
            return FALSE;
        }
    }

    return XFER_OM_SetAutoOpCodeUpgradePath(path_p);
} /* End XFER_MGR_SetAutoOpCodeUpgradePath() */

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
BOOL_T XFER_MGR_GetAutoOpCodeUpgradePath(char *path_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    XFER_OM_GetAutoOpCodeUpgradePath(path_p);

    return TRUE;
} /* End XFER_MGR_GetAutoOpCodeUpgradePath() */

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
SYS_TYPE_Get_Running_Cfg_T XFER_MGR_GetRunningAutoOpCodeUpgradePath(char *path_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    XFER_OM_GetAutoOpCodeUpgradePath(path_p);

    if (0 == strcmp(SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_PATH, path_p))
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End XFER_MGR_GetRunningAutoOpCodeUpgradePath() */

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
BOOL_T XFER_MGR_GetAutoOpCodeUpgradeFileName(char *filename_p)
{
#if (SYS_CPNT_CONFDB == TRUE)
    uint32_t                length = MAXSIZE_fileAutoUpgradeOpCodeFileName + 1;
    CONFDB_RETURN_CODE_T    res;
#endif /* SYS_CPNT_CONFDB */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

#if (SYS_CPNT_CONFDB == TRUE)
    res = CONFDB_GetUtf8Value("xfer", "auto_upgrade_path", filename_p, &length);
    if (res == CONFDB_SUCCESS)
    {
        return TRUE;
    }
#endif /* SYS_CPNT_CONFDB */

    strncpy(filename_p, SYS_ADPT_XFER_AUTO_UPGRADE_OPCODE_SEARCH_FILENAME, MAXSIZE_fileAutoUpgradeOpCodeFileName);
    filename_p[MAXSIZE_fileAutoUpgradeOpCodeFileName] = '\0';

    return  TRUE;
} /* End XFER_MGR_GetAutoOpCodeUpgradePath() */

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
void XFER_MGR_NETCFG_RifUp_CallBack(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    /* Recode the RIF is up event.
     * When the CLI provision is completed, we will use this value to decide does
     * need to check the new image.
     */
    xfer_mgr_rif_up = TRUE;

    /* If the CLI provision does not complete yet, then check will be postponed
     * to the CLI provision complete.
     */
    if (XFER_MGR_IsProvisionComplete() == FALSE)
    {
        return;
    }

    XFER_MGR_UpgradeFile(FALSE);
}
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
BOOL_T XFER_MGR_AutoDownLoad(UI8_T  *unit_list,
                             UI8_T  *destfile,
                             UI8_T  *srcfile,
                             UI32_T file_type,
                             BOOL_T is_next_boot,
                             void *cookie,
                             UI32_T ipc_message_q,
                             void   (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    UI32_T unit_id = 0;
    BOOL_T set_is_busy_flag = FALSE;

    STKTPLG_POM_GetMyUnitID(&unit_id);

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }
    /* BODY
     */
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));
    if(destfile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,destfile);
        return FALSE;
    }

    if(srcfile== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,srcfile);
        return FALSE;
    }

    if (strlen((char *)destfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,destfile);
        return FALSE;
    }

    if (strlen((char *)srcfile) > XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,srcfile);
        return FALSE;
    }

    if (FS_GenericFilenameCheck(destfile, file_type) != FS_RETURN_OK)
    {
        return FALSE;
    }

    if (FS_GenericFilenameCheck(srcfile, file_type) != FS_RETURN_OK)
    {
        return FALSE;
    }

    if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_AUTODOWNLOAD)) )== NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if ( (mtext->buf = BUFFER_MGR_Allocate()) == 0 )
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free (mtext);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }


    mtext->unit_id =        unit_id;
    mtext->cookie =         cookie;
    mtext->callback =       callback;
    mtext->file_type =      file_type;
    mtext->is_next_boot =   is_next_boot;
    mtext->ipc_message_q =  ipc_message_q;

    strcpy ((char *)mtext->destfile, (char *)destfile);
    strcpy ((char *)mtext->srcfile, (char *)srcfile);
    memcpy (mtext->unit_list, unit_list, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
    msg.mtype = XFER_MGR_COPYTOALLUNIT;
    msg.mtext = mtext;

    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
	req_msg_p->msg_type = 1;
    req_msg_p->msg_size = sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

}/* End of XFER_MGR_AutoDownLoad() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetAutoDownLoad_Status
 *------------------------------------------------------------------------
 * FUNCTION:  Get auto download status
 * INPUT   : unit_id -> unit id
 * OUTPUT  : *auto_download_status
 * RETURN  : TRUE: succes;FALSE: fail
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetAutoDownLoad_Status(UI32_T unit_id, XFER_MGR_Auto_Download_T *download_status)
{
    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }
    if (FALSE == STKTPLG_POM_UnitExist(unit_id))
    {
        return FALSE;
    }

    download_status->auto_download_status = auto_download_unit[unit_id-1].auto_download_status;
    download_status->copy_action = auto_download_unit[unit_id-1].copy_action;
    return TRUE;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetNextAutoDownLoad_Status
 *------------------------------------------------------------------------
 * FUNCTION:  Get auto download status
 * INPUT   : *unit_id -> unit id
 * OUTPUT  : *auto_download_status
 * RETURN  : TRUE: succes;FALSE: fail
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T XFER_MGR_GetNextAutoDownLoad_Status(UI32_T *unit_id, XFER_MGR_Auto_Download_T *download_status)
{
    UI32_T unit = *unit_id;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }
    if (unit >= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    unit++;
    while (FALSE == STKTPLG_POM_UnitExist(unit))
    {
        unit++;
        if (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        {
            return FALSE;
        }
    }

    *unit_id = unit;
    download_status->auto_download_status = auto_download_unit[unit-1].auto_download_status;
    download_status->copy_action = auto_download_unit[unit-1].copy_action;
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XFER_MGR_GetFileCopyStatusString
 *-------------------------------------------------------------------------
 * PURPOSE  : Get descriptive status.
 * INPUT    : XFER_MGR_FileCopyStatus_T     status
 * OUTPUT   : UI8_T                         *status_string
 * RETURN   : None
 * NOTE     : the buffer size of status string should be MAXSIZE_fileCopyTftpErrMsg
 *-------------------------------------------------------------------------*/
void XFER_MGR_GetFileCopyStatusString(XFER_MGR_FileCopyStatus_T status, UI8_T *status_string)
{
    switch(status)
    {
        case XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR:
            XFER_MGR_GetTftpErrorMsg(status_string);
            break;

        case XFER_MGR_FILE_COPY_FILE_NOT_FOUND:
            strcpy((char *)status_string, "File not found");
            break;

        case XFER_MGR_FILE_COPY_SERVER_PERMISSION_DENIED:
            strcpy((char *)status_string, "Server permission denied");
            break;

        case XFER_MGR_FILE_COPY_STORAGE_FULL:
            strcpy((char *)status_string, "Storage full");
            break;

        case XFER_MGR_FILE_COPY_TFTP_ILLEGAL_OPERATION:
            strcpy((char *)status_string, "Illegal operation");
            break;

        case XFER_MGR_FILE_COPY_TFTP_UNKNOWN_TRANSFER_ID:
            strcpy((char *)status_string, "Unknown transfer id");
            break;

        case XFER_MGR_FILE_COPY_TFTP_FILE_EXISTED:
            strcpy((char *)status_string, "File existed");
            break;

        case XFER_MGR_FILE_COPY_TFTP_NO_SUCH_USER:
            strcpy((char *)status_string, "No such user");
            break;

        case XFER_MGR_FILE_COPY_TIMEOUT:
            strcpy((char *)status_string, "Timeout");
            break;

        case XFER_MGR_FILE_COPY_TFTP_SEND_ERROR:
            strcpy((char *)status_string, "Send error");
            break;

        case XFER_MGR_FILE_COPY_TFTP_RECEIVE_ERROR:
            strcpy((char *)status_string, "Receive error");
            break;

        case XFER_MGR_FILE_COPY_TFTP_SOCKET_OPEN_ERROR:
            strcpy((char *)status_string, "Socket open error");
            break;

        case XFER_MGR_FILE_COPY_TFTP_SOCKET_BIND_ERROR:
            strcpy((char *)status_string, "Socket bind error");
            break;

        case XFER_MGR_FILE_COPY_TFTP_USER_CANCELED:
            strcpy((char *)status_string, "User canceled operation");
            break;

        case XFER_MGR_FILE_COPY_COMPLETED:
            strcpy((char *)status_string, "Complete");
            break;

        case XFER_MGR_FILE_COPY_SET_STARTUP_ERROR:
            strcpy((char *)status_string, "Set startup file error");
            break;

        case XFER_MGR_FILE_COPY_FILE_SIZE_EXCEED:
            strcpy((char *)status_string, "File size exceed");
            break;

        case XFER_MGR_FILE_COPY_MAGIC_WORD_ERROR:
            strcpy((char *)status_string, "Image magic word error");
            break;

        case XFER_MGR_FILE_COPY_SUCCESS:
            strcpy((char *)status_string, "Success");
            break;

        case XFER_MGR_FILE_COPY_ERROR:
            strcpy((char *)status_string, "Error");
            break;

        case XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR:
            strcpy((char *)status_string, "Header checksum error");
            break;

        case XFER_MGR_FILE_COPY_IMAGE_CHECKSUM_ERROR:
            strcpy((char *)status_string, "Image checksum error");
            break;

        case XFER_MGR_FILE_COPY_IMAGE_TYPE_ERROR:
            strcpy((char *)status_string, "Image type error");
            break;

        case XFER_MGR_FILE_COPY_PROJECT_ID_ERROR:
            strcpy((char *)status_string, "Project ID error");
            break;

        case XFER_MGR_FILE_COPY_BUSY:
            strcpy((char *)status_string, "Busy");
            break;

        case XFER_MGR_FILE_COPY_PARA_ERROR:
            strcpy((char *)status_string, "Parameter error");
            break;

        case XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH:
            strcpy((char *)status_string, "Flash programming completed");
            break;

        case XFER_MGR_FILE_COPY_WRITE_FLASH_ERR:
            strcpy((char *)status_string, "Flash programming error");
            break;

        case XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING:
            strcpy((char *)status_string, "Flash programming started");
            break;

        case XFER_MGR_FILE_COPY_READ_FILE_ERROR:
            strcpy((char *)status_string, "Read file error");
            break;

        case XFER_MGR_FILE_COPY_UNKNOWN:
            strcpy((char *)status_string, "Unknown error");
            break;

        case XFER_MGR_FILE_COPY_FILE_NUM_EXCEED:
            strcpy((char *)status_string, "The number of files of the given type exceeds the maximum number.");
            break;

        case XFER_MGR_FILE_COPY_SAME_VERSION:
            strcpy((char *)status_string, "Same version number. Not updated.");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT1:
            strcpy((char *)status_string, "Synchronizing to Unit1");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT2:
            strcpy((char *)status_string, "Synchronizing to Unit2");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT3:
            strcpy((char *)status_string, "Synchronizing to Unit3");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT4:
            strcpy((char *)status_string, "Synchronizing to Unit4");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT5:
            strcpy((char *)status_string, "Synchronizing to Unit5");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT6:
            strcpy((char *)status_string, "Synchronizing to Unit6");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT7:
            strcpy((char *)status_string, "Synchronizing to Unit7");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT8:
            strcpy((char *)status_string, "Synchronizing to Unit8");
            break;

        default:
            strcpy((char *)status_string, "Unclassified error");
            break;
    }
}

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
#define XFER_MGR_DUMP_FAIL_REASON SYSFUN_Debug_Printf("XFER: Failed in line %d, function: %s, in %s\r\n", __LINE__, __FUNCTION__, __FILE__);

BOOL_T XFER_MGR_AutoConfigToUnit(UI8_T  *file_name,
                                 void *cookie,
                                 UI32_T ipc_message_q,
                                 void   (*callback) (void *cookie, UI32_T status))
{
    /* LOCAL VARIABLE DECLARATIONS
     */
    XFER_Msg_T msg;
    Mtext_T *mtext;
    UI32_T unit_id = 0;
    BOOL_T set_is_busy_flag = FALSE;

    STKTPLG_POM_GetMyUnitID(&unit_id);

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        XFER_MGR_DUMP_FAIL_REASON;
        return FALSE;
    }
    /* BODY
     */
    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));
    if(file_name== NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,file_name);
        XFER_MGR_DUMP_FAIL_REASON;
        return FALSE;
    }

    if (strlen((char *)file_name) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,file_name);
        XFER_MGR_DUMP_FAIL_REASON;
        return FALSE;
    }

    if (FS_GenericFilenameCheck(file_name, FS_FILE_TYPE_CONFIG) != FS_RETURN_OK)
    {
        XFER_MGR_DUMP_FAIL_REASON;
        return FALSE;
    }

    if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        XFER_MGR_DUMP_FAIL_REASON;
        return FALSE;
    }

    if((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_AUTOCONFIGTOUNIT)) )== NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        XFER_MGR_DUMP_FAIL_REASON;
        return FALSE;
    }

    if ( (mtext->buf = BUFFER_MGR_Allocate()) == 0 )
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free (mtext);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_LOCALTOUNIT_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        XFER_MGR_DUMP_FAIL_REASON;
        return FALSE;
    }


    mtext->unit_id =        unit_id;
    mtext->file_type =      FS_FILE_TYPE_CONFIG;
    mtext->is_next_boot =   TRUE;
    mtext->cookie =         cookie;
    mtext->callback =       callback;
    mtext->ipc_message_q =  ipc_message_q;

    strcpy ((char *)mtext->destfile, (char *)file_name);
    strcpy ((char *)mtext->srcfile, (char *)file_name);
    msg.mtype = XFER_MGR_CONFIGTOUNIT;
    msg.mtext = mtext;

    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
	req_msg_p->msg_type = 1;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        XFER_MGR_DUMP_FAIL_REASON;
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

}/* End of XFER_MGR_AutoConfigToUnit() */

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
void XFER_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    return;
}

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
void XFER_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    return;
}


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
void XFER_MGR_GetInfoDownloadLength(UI32_T *out_len)
{
    *out_len = info.tftp_download_length;
    return;
}/* End of XFER_MGR_GetInfoDownloadLength */
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
BOOL_T XFER_MGR_SetFileCopyActionFlag(UI32_T action)
{
    if (info.is_busy)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_SETFILECOPYACTION_FUNC_NO,
            EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }
    file_copy_mgt_entry.action = action;

    if(action == VAL_fileCopyAction_copy)
    {
        is_file_copy_mgt_used = TRUE;
    }
    else if (action == VAL_fileCopyAction_notCopying)
    {
        is_file_copy_mgt_used = FALSE;
    }
    else
        return FALSE;

    return TRUE;
} /* End of XFER_MGR_SetFileCopyActionFlag */

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
BOOL_T XFER_MGR_SetFileCopyStatusFlag(UI32_T status)
{
    is_file_copy_mgt_used = FALSE;
    file_copy_mgt_entry.status = status;

    return TRUE;
} /* End of XFER_MGR_SetFileCopyStatusFlag */

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetPartialProvisionStatus
 *------------------------------------------------------------------------
 * FUNCTION: set partial provision atatus
 * INPUT   : status         -- file copy status
 *           ipc_message_q  -- the key of CSC group message queue
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void
XFER_MGR_SetPartialProvisionStatus(
    UI32_T status)
{
    if (NULL != file_copy_mgt_entry.callback)
    {
        if (file_copy_mgt_entry.ipc_message_q == SYS_BLD_XFER_GROUP_IPCMSGQ_KEY)
        {
            (file_copy_mgt_entry.callback) (file_copy_mgt_entry.cookie, status);
            (file_copy_mgt_entry.callback) (file_copy_mgt_entry.cookie, XFER_MGR_FILE_COPY_COMPLETED);
        }
        else
        {
            SYS_CALLBACK_MGR_AnnounceXferResultCallback(SYS_MODULE_XFER,
                                                        file_copy_mgt_entry.ipc_message_q,
                                                        (void *)(file_copy_mgt_entry.callback),
                                                        file_copy_mgt_entry.cookie,
                                                        status);
            SYS_CALLBACK_MGR_AnnounceXferResultCallback(SYS_MODULE_XFER,
                                                        file_copy_mgt_entry.ipc_message_q,
                                                        (void *)(file_copy_mgt_entry.callback),
                                                        file_copy_mgt_entry.cookie,
                                                        XFER_MGR_FILE_COPY_COMPLETED);
        }
    }

    file_copy_mgt_entry.status = status;
    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    is_file_copy_mgt_used = FALSE;
    BUFFER_MGR_Free(file_copy_mgt_entry.buf);
}
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

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
BOOL_T XFER_MGR_SetFileCopyOamPorts(UI8_T *in_portlist_p)
{
    UI32_T  i=0;
    BOOL_T  portlist_is_null=TRUE;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    /* check the bit in portlist has '1' or not */
    for(i=0;i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        if(in_portlist_p[i] == 0)
        {
            portlist_is_null = TRUE;
        }
        else
        {
            portlist_is_null = FALSE;
            break;
        }
    }

    /* the port doesn't specify in port list. */
    if(portlist_is_null== TRUE)
    {
        return FALSE;
    }

    memcpy(file_copy_mgt_entry.oam_portlist,
        in_portlist_p,
        SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    return TRUE;
}/* End of XFER_MGR_SetFileCopyOamPorts() */
#endif
#endif
#endif
#endif

typedef union XFER_MGR_MSG_U
{
    XFER_Msg_T  msg;
} XFER_MGR_MSG_T;

#define CSC_XFER_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(XFER_MGR_MSG_T)

/*
 * LOCAL SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Main
 * -------------------------------------------------------------------------
 * FUNCTION: XFER main task routine
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void XFER_MGR_Main(void)
{
    UI32_T  event_var;
    UI32_T  wait_events;
    UI32_T  rcv_events;
    UI32_T  timeout;
    UI32_T  ret_value, ret;
    SYS_TYPE_Stacking_Mode_T got_operation_mode;

    XFER_Msg_T  msg;

    UI8_T msg_space_p[sizeof(SYSFUN_Msg_T)+sizeof(XFER_DNLD_Msg_T)];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;

    /* Prepare waiting event and init. event var. */
    wait_events = XFER_MGR_EVENT_MSG |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                  SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                  XFER_MGR_EVENT_ENTER_TRANSITION;
    event_var = 0;

    if(SYSFUN_CreateMsgQ(SYS_BLD_CSC_XFER_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &xfer_mgr_task_msgQ_id)
        != SYSFUN_OK)
    {
        while(1);
    }


   /*Body*/
    while( 1 )
    {
        /* Check timer event and message event */
        if (event_var != 0)
        {    /* There is some message in message queue  */
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;

        if ((ret_value=SYSFUN_ReceiveEvent (wait_events, SYSFUN_EVENT_WAIT_ANY,
                                 timeout, &rcv_events))!=SYSFUN_OK)
        {
            if (ret_value != SYSFUN_RESULT_NO_EVENT_SET)
            {
                /*  Log to system : unexpect return value   */
                ;
            }
        }
        event_var |= rcv_events;

        if (event_var==0)
        {
            /*  Log to system : ERR--Receive Event Failure */
            continue;
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_XFER);
            event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        /* Get operation mode from MGR */
        got_operation_mode = XFER_MGR_GetOperationMode();

        if (got_operation_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {

    		/* task in transition mode, should clear resource (msgQ) in task */
    		while( SYSFUN_ReceiveMsg( xfer_mgr_task_msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(XFER_Msg_T), req_msg_p)
    		       ==SYSFUN_OK)
            {
                memcpy(&msg,req_msg_p->msg_buf,sizeof(XFER_Msg_T));
                L_MM_Free ( msg.mtext );
            }   /*  end of while */

            if (event_var & XFER_MGR_EVENT_ENTER_TRANSITION )
                is_transition_done = TRUE;  /* Turn on the transition done flag */
                event_var = 0;
                continue;
        }

        if (event_var & XFER_MGR_EVENT_MSG)
        {
/*
            if(SYSFUN_ReceiveMsg( xfer_mgr_task_msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(Msg_T), req_msg_p)
                         == SYSFUN_RESULT_NO_MESSAGE)
*/
            ret = SYSFUN_ReceiveMsg( xfer_mgr_task_msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT,
                     CSC_XFER_PMGR_MSGBUF_MAX_REQUIRED_SIZE, req_msg_p);

           if (ret  == SYSFUN_RESULT_NO_MESSAGE)
            {

                event_var ^= XFER_MGR_EVENT_MSG;
            }
            else /* receive a message from message queue, process the message  */
            {


                memcpy(&msg,req_msg_p->msg_buf,sizeof(XFER_Msg_T));
                switch ( msg.mtype )
                {
                    case XFER_MGR_COPYFILE:
                       XFER_MGR_CopyFile_Syn ( msg.mtext);
                       break;

                    case XFER_MGR_REMOTETOSTREAM:
                       XFER_MGR_RemoteToStream_Syn (msg.mtext);
                       break;

                    case XFER_MGR_STREAMTOLOCAL:
                       XFER_MGR_StreamToLocal_Syn (msg.mtext);
                       break;

                    case XFER_MGR_STREAMTOREMOTE:
                       XFER_MGR_StreamToRemote_Syn (msg.mtext);
                       break;

                    case XFER_MGR_COPYUNITFILE:
                       XFER_MGR_UnitToLocal_Syn (msg.mtext);
                       break;

                    case XFER_MGR_COPYFILEUNIT:
                       XFER_MGR_LocalToUnit_Syn (msg.mtext);
                       break;

                    case XFER_MGR_WRITEFILE:
                        XFER_MGR_WriteFile_Syn(msg.mtext);
				        BUFFER_MGR_Free(msg.mtext->buf);
                        break;

                    case XFER_MGR_COPYTOALLUNIT:
                       XFER_MGR_AutoDownLoad_Syn (msg.mtext);
                       XFER_MGR_CallBack(msg.mtext, XFER_MGR_FILE_COPY_COMPLETED);
                       BUFFER_MGR_Free(msg.mtext->buf);
                       break;

                    case XFER_MGR_CONFIGTOUNIT:
                       XFER_MGR_AutoConfigToUnit_Syn (msg.mtext);
                       break;

                #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
                    case XFER_MGR_UPGRADEFILE:
                       XFER_MGR_UpgradeFile_Syn(msg.mtext);
                       break;
                #endif

                    default:
                        break;
               } /* End of switch */

                info.tftp_status            = XFER_DNLD_TFTP_UNDEF_ERROR;
                info.current_tftp_status    = XFER_DNLD_TFTP_UNDEF_ERROR;
                info.is_end_with_provision  = FALSE;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
                info.is_partial_provision   = FALSE;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

                info.is_next_boot           = FALSE;
                info.tftp_download_length   = 0;
               L_MM_Free ( msg.mtext );
               XFER_MGR_Transaction_End(TRUE);
               SYSFUN_Sleep(200);
          }
      } /* if (event_var & XFER_MGR_EVENT_MSG) */
   } /* End of while */
} /* End of XFER_MGR_Main */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_InitAutoDownloadStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Init auto download status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-----------------------------------------------------------------*/
static void XFER_MGR_InitAutoDownloadStatus()
{
    UI32_T i = 0;

    for (i = 1; i <=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
       auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_UNKNOWN;
       auto_download_unit[i-1].copy_action = VAL_fileCopyAction_notCopying;
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_InitTftpMgrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will Init TftpMgrEntry
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-----------------------------------------------------------------*/
static void XFER_MGR_InitTftpMgrEntry(void)
{
    memset (&TftpMgrEntry, 0, sizeof(XFER_MGR_TftpMgtEntry_T));
    TftpMgrEntry. tftpStatus = XFER_MGR_BUSY;
    TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
    TftpMgrEntry.tftpFileType = FS_TYPE_OPCODE_FILE_TYPE;
    return;
}/* End of XFER_MGR_InitTftpMgrEntry() */

#if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
static BOOL_T XFER_MGR_IsCheckProjectId(void)
{
    return (XFER_MGR_CheckProjectId);
} /* End of XFER_MGR_IsCheckProjectId() */
#endif

static BOOL_T XFER_MGR_IsCheckImageVersion(void)
{
    return (XFER_MGR_CheckImageVersion);
} /* End of XFER_MGR_IsCheckImageVersion() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ConvertModeToDownloadFlag
 *------------------------------------------------------------------------
 * FUNCTION: The function checks the mode is download
 * INPUT   : mode - XFER_MGR_Mode_T
 * OUTPUT  : none
 * RETURN  : TRUE  - download mode
 *           FALSE - not download mode
 * NOTE    :
 *------------------------------------------------------------------------*/
static BOOL_T
XFER_MGR_ConvertModeToDownloadFlag(
    XFER_MGR_Mode_T mode)
{
    UI32_T mode_transfer_table[] = {
#define MODE(mode, remoteTransfer) [mode] = remoteTransfer,
            XFER_MGR_MODE_TABLE
#undef MODE
        };

    if (mode >= (sizeof(mode_transfer_table)/sizeof(mode_transfer_table[0])))
    {
        /* error case, should not happened */
        return FALSE;
    }

    return (XFER_MGR_REMOTE_TRANSFER_DOWNLOAD == mode_transfer_table[mode]) ? TRUE : FALSE;
}

#if defined(JBOS)
void XFER_MGR_SetCheckImageType(BOOL_T CheckImageType)
{
    XFER_MGR_CheckProjectId = CheckImageType;
    return ;
} /* End of XFER_MGR_SetCheckImageType() */
#endif

static BOOL_T XFER_MGR_IsDebugFlagOn(void)
{
    return (debug_flag);
} /* End of XFER_MGR_IsDebugFlagOn() */

/*------------------------------------------------------------------------
* ROUTINE NAME - XFER_MGR_TftpDnld_CallBack
*------------------------------------------------------------------------
* FUNCTION: The function is a call back function is get tftpDnld status
* INPUT   : none
* OUTPUT  :
* RETURN  : none
* NOTE    : none
*------------------------------------------------------------------------*/
static void XFER_MGR_TftpDnld_CallBack (UI32_T status, UI32_T download_length)
{
    UI8_T tftp_error_msg[MAXSIZE_fileCopyTftpErrMsg + 1];

    memset(tftp_error_msg, 0, sizeof(tftp_error_msg));

    switch (status)
    {
        case XFER_DNLD_TFTP_BUF_SIZE_EXCEEDS:
            status = XFER_MGR_FILE_COPY_FILE_SIZE_EXCEED;
            break;

        case XFER_DNLD_TFTP_UNDEF_ERROR:
            if(XFER_DNLD_GetTftpErrorMsg(tftp_error_msg))
            {
                memcpy(file_copy_mgt_entry.tftp_error_msg, tftp_error_msg, strlen((char *)tftp_error_msg));
                file_copy_mgt_entry.tftp_error_msg[strlen((char *)tftp_error_msg)] = '\0';
            }
            status = XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR;
            break;

        case XFER_DNLD_TFTP_FILE_NOT_FOUND:
            status = XFER_MGR_FILE_COPY_FILE_NOT_FOUND;
            break;

        case XFER_DNLD_TFTP_ACCESS_VIOLATION:
            status = XFER_MGR_FILE_COPY_SERVER_PERMISSION_DENIED;
            break;

        case XFER_DNLD_TFTP_DISK_FULL:
            status = XFER_MGR_FILE_COPY_STORAGE_FULL;
            break;

        case XFER_DNLD_TFTP_TIMEOUT:
            status = XFER_MGR_FILE_COPY_TIMEOUT;
            break;

        default:
            break;
    }

    info.tftp_status = info.current_tftp_status;
    info.current_tftp_status = status;
    info.tftp_download_length = download_length;
    return;
}/* End of XFER_MGR_TftpDnld_CallBack() */

static void XFER_MGR_CallBack ( Mtext_T *mtext, XFER_MGR_FileCopyStatus_T  status)
{
    if (mtext->ipc_message_q == SYS_BLD_XFER_GROUP_IPCMSGQ_KEY)
    {
        if ( mtext->callback)
        {
            (*mtext->callback) ( mtext->cookie, status );
        }
    }
    else
    {
        SYS_CALLBACK_MGR_AnnounceXferResultCallback(SYS_MODULE_XFER, mtext->ipc_message_q , (void *)mtext->callback, mtext->cookie, status);
    }

} /* End of XFER_MGR_CallBack() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Transaction_Begin
 *------------------------------------------------------------------------
 * FUNCTION: Begins a transaction
 * INPUT   : immediately    -  do file operation immediately and not wait other
 *                             file operation
 *                             TRUE, returns FALSE if have other transaction
 *                             began
 *                             FALSE, always returns TRUE
 * OUTPUT  : busy_p         -  returns the global is_busy monitor status
 *                             TRUE, the global is_busy be changed from FALSE
 *                             to TRUE
 * RETURN  : TRUE(succeeds) / FALSE(fails)
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T XFER_MGR_Transaction_Begin(BOOL_T immediately, BOOL_T *busy_p)
{
    *busy_p = FALSE;

    if (TRUE == immediately)
    {
        if ( TRUE == info.is_busy )
        {
            return FALSE;
        }
    }

    if (FALSE == info.is_busy)
    {
        info.is_busy = TRUE;
        *busy_p = TRUE;
    }

    return TRUE;
} /* End of XFER_MGR_Transaction_Begin() */

/*-----------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Transaction_End
 *-----------------------------------------------------------------
 * FUNCTION: Ends the current transaction
 * INPUT   : clear_busy_flag  -  TRUE, clear the global is_busy flag
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-----------------------------------------------------------------
 */
static void XFER_MGR_Transaction_End(BOOL_T clear_busy_flag)
{
    if (TRUE == clear_busy_flag)
    {
        info.is_busy = FALSE;
    }
}/* End of XFER_MGR_Transaction_End() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFile_Syn
 *------------------------------------------------------------------------
 * FUNCTION: The function is remote file copy to loacl file
 * INPUT   : XFER_MGR_Msg_T *mtext
 * OUTPUT  :
 * RETURN  : none
 * NOTE    : Must free mtext
 *------------------------------------------------------------------------*/
static void XFER_MGR_CopyFile_Syn (Mtext_T *mtext)
{
    UI32_T              unit_id;
    UI32_T              read_count;
    FS_File_Attr_T      file_attr;
    XFER_DNLD_Msg_T     send_msg;
    XFER_DNLD_Mtext_T   *send_text;
    UI32_T              ret;
    UI8_T               startup_file_name[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME + 1];
    char                original_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME + 1] = {0};

    /*BODY
     */
    STKTPLG_POM_GetMyUnitID(&unit_id);
    if((send_text = L_MM_Malloc(sizeof(XFER_DNLD_Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_COPYFILE_SYN)) ) == NULL)
    {
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        mtext->status = file_copy_mgt_entry.status;
        XFER_MGR_SendFileCopyTrap(mtext);
        is_file_copy_mgt_used = FALSE;
        BUFFER_MGR_Free(mtext->buf);
        return;
    }

    send_msg.is_download = XFER_MGR_ConvertModeToDownloadFlag(mtext->mode);
    send_text->tftp_retry_times = XFER_OM_GetTftpRetryTimes();
    send_text->tftp_timeout     = XFER_OM_GetTftpTimeout();
    memcpy( &(send_text->server_ip), &(mtext->server_ip), sizeof(send_text->server_ip));
    send_text->buf =            mtext->buf;
    send_msg.server_type =  mtext->server_type;
    send_text->action = XFER_DNLD_ACTION_GET_WHOLE_FILE;

    XFER_MGR_CopyUserInfoByServerType(mtext->server_type, mtext->username, mtext->password, send_text->username, send_text->password);

    send_msg.mtext = send_text;

    switch (mtext->mode)
    {
        case XFER_MGR_LOCAL_TO_LOCAL:
        case XFER_MGR_LOCAL_TO_STARTUP:
        case XFER_MGR_STARTUP_TO_LOCAL:
            if (   (mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE)
                || (mtext->file_type == FS_FILE_TYPE_CONFIG)
                )
            {
                if(XFER_MGR_IsFileNumExceededByType(mtext->destfile, mtext->file_type) == TRUE)
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_FILE_NUM_EXCEED);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_FILE_NUM_EXCEED;
                    break;
                }
            }

            file_attr.file_type_mask = FS_FILE_TYPE_MASK_ALL;
            strcpy ( (char *)file_attr.file_name, (char *)mtext->srcfile);
            if( FS_GetFileInfo(unit_id, &file_attr)!= FS_RETURN_OK)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                break;
            }

            if ( FS_ReadFile( unit_id, file_attr.file_name,  mtext->buf, SYS_ADPT_MAX_FILE_SIZE,
                 &read_count) != FS_RETURN_OK)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                break;
            }
            mtext->length=read_count;

            XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);
            ret = FS_WriteFile (unit_id,mtext->destfile, (UI8_T *)"Xfer", file_attr.file_type, mtext->buf, read_count,0);
            if (ret == FS_RETURN_OK)

            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
                if(TRUE == mtext->is_next_boot &&
                   FS_RETURN_OK != FS_SetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, mtext->destfile))
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SET_STARTUP_ERROR);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SET_STARTUP_ERROR;
                }
                else
                {
                    /* if the destination file is current startup file then sync to slave
                     */
                    if (    (FS_RETURN_OK == FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
                        &&  (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile))
                       )
                    {
                        /* save original source filename cause it will be change when sync to slave
                         */
                        strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                        original_filename[sizeof(original_filename)-1] = '\0';

                        /* sync to slaves
                         */
                        mtext->unit_id = unit_id;
                        strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                        mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                        XFER_MGR_AutoDownLoad_Syn(mtext);

                        /* revert to original source filename
                         */
                        strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                        mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                    }

                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                }
            }
            else
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            }
            break;

        case XFER_MGR_STARTUP_TO_STREAM:
            strcpy((char *)mtext->buf, CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD);

            if (FS_RETURN_OK != FS_ReadFile(unit_id, mtext->srcfile,
                    mtext->buf + strlen(CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD),
                    SYS_ADPT_MAX_FILE_SIZE - strlen(CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD),
                    &read_count))
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                break;
            }
            mtext->length=read_count;

            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            XFER_MGR_SendFileCopyTrap(mtext);
            is_file_copy_mgt_used = FALSE;
            L_MM_Free(send_msg.mtext);
            CLI_PMGR_Notify_EnterTransitionMode((void *)mtext->buf);
            return;

        case XFER_MGR_STREAM_TO_STARTUP:

            if(XFER_MGR_IsFileNumExceededByType(mtext->destfile, FS_FILE_TYPE_CONFIG) == TRUE)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_FILE_NUM_EXCEED);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_FILE_NUM_EXCEED;
                break;
            }

            /* collect running-config
             */
            if (CLI_MGR_RUNCFG_RETURN_NO_ENOUGH_MEMORY ==
                CLI_PMGR_Get_RunningCfg(mtext->buf, SYS_ADPT_MAX_FILE_SIZE))
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                break;
            }

            mtext->length = strlen((char *)mtext->buf);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);
            ret = FS_WriteFile(unit_id, mtext->destfile, (UI8_T *)"Xfer",
                      mtext->file_type, mtext->buf, mtext->length, 0);

            if (ret == FS_RETURN_OK)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);

                if (TRUE == mtext->is_next_boot &&
                    FS_RETURN_OK != FS_SetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, mtext->destfile))
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SET_STARTUP_ERROR);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SET_STARTUP_ERROR;
                }
                else
                {
                    /* if the destination file is current startup file then sync to slave
                     */
                    if ((FS_RETURN_OK == FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, startup_file_name))
                            && (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile)))
                    {
                        /* save original source filename cause it will be change when sync to slave
                         */
                        strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                        original_filename[sizeof(original_filename)-1] = '\0';

                        /* sync to slaves
                         */
                        mtext->unit_id = unit_id;
                        strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                        mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                        XFER_MGR_AutoDownLoad_Syn(mtext);

                        /* revert to original source filename
                         */
                        strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                        mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                    }

                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                }
            }
            else
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            }
            break;

        case XFER_MGR_USBDISK_TO_LOCAL:

            if(mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE
#if (SYS_CPNT_XFER_DOWNLOAD_DIAG == TRUE)
               || mtext->file_type == FS_FILE_TYPE_DIAG
#endif
              )
            {
                /* For auto upgrade code, no need to check the number of runtime
                 * image. Auto upgrade code can handle this issue.
                 */
                if (
#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
                    XFER_MGR_UpgradeFileProgress_CallBack != mtext->callback &&
#endif
                    XFER_MGR_IsFileNumExceededByType(mtext->destfile, mtext->file_type) == TRUE)
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_FILE_NUM_EXCEED);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_FILE_NUM_EXCEED;
                    break;
                }
            }
            /* read from usbdisk */
            int fd_dst;
            UI32_T len = 0, offset = 0;
            UI32_T block_size = USB_FILEOPER_MGR_BLOCKSIZE;
            UI32_T max_file_size = SYS_ADPT_MAX_FILE_SIZE;
            char src_fullpathname[USB_MAXSIZE_DestFile+10]="/mnt/usb/";
            strncat(src_fullpathname, (char *)mtext->srcfile, strlen((char *)mtext->srcfile));
            src_fullpathname[strlen(src_fullpathname)]='\0';
            fd_dst = open(src_fullpathname,O_RDONLY);
            if(fd_dst < 0)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                break;
            }

            do
            {
                len = read(fd_dst, &(mtext->buf[offset]), block_size);
                if(len < 0)
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                    close(fd_dst);
                    break;
                }

                offset += len;

                if( (len < block_size) || (0 == len))
                {
                    break;
                }

            }while(offset < max_file_size);

            if(offset > max_file_size)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                close(fd_dst);
                break;
            }

            close(fd_dst);
            if(mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE)
            {
                /* check download opcode file (both mainboard and option module)
                 * check item: MagicWord, ImageChecksum, and HeaderChecksum
                 */
                if (FALSE == XFER_MGR_CheckImageHeaderValidation(mtext,offset))
                {
                    break;
                }
            }

            XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);
            mtext->length=offset;
            ret = FS_WriteFile (unit_id,mtext->destfile, (UI8_T *)"Xfer", mtext->file_type, mtext->buf, offset,0);
            if (ret == FS_RETURN_OK)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
                if(TRUE == mtext->is_next_boot &&
                   FS_RETURN_OK != FS_SetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, mtext->destfile))
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SET_STARTUP_ERROR);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SET_STARTUP_ERROR;
                }
                else
                {
                    /* if the destination file is current startup file then sync to slave
                     */
                    if (    (FS_RETURN_OK == FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
                        &&  (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile))
                       )
                    {
                        /* save original source filename cause it will be change when sync to slave
                         */
                        strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                        original_filename[sizeof(original_filename)-1] = '\0';

                        /* sync to slaves
                         */
                        mtext->unit_id = unit_id;
                        strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                        mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                        XFER_MGR_AutoDownLoad_Syn(mtext);

                        /* revert to original source filename
                         */
                        strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                        mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                    }

                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                }
            }
            else
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            }
            break;

        case XFER_MGR_REMOTE_TO_LOCAL:
        case XFER_MGR_REMOTE_TO_STARTUP:
            if(mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE)
            {
                /* For auto upgrade code, no need to check the number of runtime
                 * image. Auto upgrade code can handle this issue.
                 */
                if (
#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
                    XFER_MGR_UpgradeFileProgress_CallBack != mtext->callback &&
#endif
                    XFER_MGR_IsFileNumExceededByType(mtext->destfile, mtext->file_type) == TRUE)
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_FILE_NUM_EXCEED);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    TftpMgrEntry.tftpStatus = XFER_MGR_FILE_COPY_FILE_NUM_EXCEED;
                    TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_FILE_NUM_EXCEED;
                    break;
                }
            }
            else if (mtext->file_type == FS_FILE_TYPE_CONFIG)
            {
                if (XFER_MGR_IsFileNumExceededByType(mtext->destfile, mtext->file_type) == TRUE)
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_FILE_NUM_EXCEED);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    TftpMgrEntry.tftpStatus = XFER_MGR_FILE_COPY_FILE_NUM_EXCEED;
                    TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_FILE_NUM_EXCEED;
                    break;
                }
            }

            send_text->uploadLength =  SYS_ADPT_MAX_FILE_SIZE;
            strcpy((char *)send_text->filename, (char *)mtext->srcfile);
            XFER_DNLD_tftp(&send_msg);
            XFER_MGR_WaitForTransmitCompleted();

            if ( info.tftp_status != XFER_DNLD_TFTP_SUCCESS)
            {
                XFER_MGR_CallBack(mtext, info.tftp_status);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                file_copy_mgt_entry.status = info.tftp_status;
                break;
            }
            mtext->length=info.tftp_download_length;

            /* allow 3 file type in 2 different copy file procedure,
             * 1. runtime image & diag image | if () { }
             * 2. config file                | else if () { }
             * 3. loader image               | else if () { }
             */
            if (   mtext->file_type == FS_FILE_TYPE_KERNEL ||
                   mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE
#if (SYS_CPNT_XFER_DOWNLOAD_DIAG == TRUE)
                || mtext->file_type == FS_FILE_TYPE_DIAG
#endif
               )
            {
                /* check download opcode file (both mainboard and option module)
                 * check item: MagicWord, ImageChecksum, and HeaderChecksum
                 */

                if (FALSE == XFER_MGR_CheckImageHeaderValidation(mtext,info.tftp_download_length))
                {
                    break;
                }

                /* 1. if the destination file is current startup file, sync to slave, write opcode to "master + slaves"
                 * 2. else write new opcode to "master only"
                 */
                if (mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE)
                {
                    #if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
                    image_header_t *imghdr;
                    #endif
                    UI8_T old_mainboard_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};
                    UI8_T new_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};

                    #if (SYS_CPNT_ONIE_SUPPORT==TRUE)
                    if (XFER_MGR_GetSWVersionFromONIEInstallerStream(mtext->buf, new_version)==FALSE)
                    {
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_MAGIC_WORD_ERROR);
                        TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_MAGIC_WORD_ERROR;
                    }
                    #else
                    imghdr = (image_header_t *)mtext->buf;
                    memcpy(new_version, imghdr->ih_ver, SYS_ADPT_FW_VER_STR_LEN+1);
                    #endif

                    if( (XFER_MGR_IsCheckImageVersion() == TRUE)
                        && XFER_MGR_Get_SWFile_Version(unit_id, old_mainboard_version, FS_TYPE_OPCODE_FILE_TYPE)
                        && (0 == strcmp((char *)old_mainboard_version, (char *)new_version)) )
                    {

                        if ((FS_RETURN_OK == FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
                            && (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile))
                           )
                        {
                            /* save original source filename cause it will be change when sync to slave
                             */
                            strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                            original_filename[sizeof(original_filename)-1] = '\0';

                            /* sync to slaves
                             */
                            mtext->unit_id = unit_id;
                            strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                            mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                            XFER_MGR_AutoDownLoad_Syn(mtext);

                            /* revert to original source filename
                             */
                            strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                            mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                        }

                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SAME_VERSION);
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SAME_VERSION;
                        TftpMgrEntry.tftpStatus = XFER_MGR_SUCCESS;
                        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                        break;
                    }

                    if (FALSE == XFER_MGR_IsOpcodeVersionAllowByLicense((char *)new_version))
                    {
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_OP_CODE_VERSION_NOT_SUPPORTED_BY_LICENSE);
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_OP_CODE_VERSION_NOT_SUPPORTED_BY_LICENSE;
                        TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                        break;
                    }
#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
                    if(TRUE == mtext->auto_assign_dest_runtime_filename)
                    {
                        UI32_T              fs_drive;
                        FS_File_Attr_T      auto_file_attr;

                        /* Removes the image that is not a startup file.
                         * Decide the filename of the new upgrade file.
                         * The default filename of the new upgrade file is op1.bix.
                         */
                        strcpy((char *)mtext->destfile, XFER_MGR_AUTO_UPGRADE_OPCODE_DEFAULT_FILENAME);

                        memset(&auto_file_attr, 0, sizeof(FS_File_Attr_T));
                        auto_file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_TYPE_OPCODE_FILE_TYPE);

                        fs_drive = unit_id;

                        while( FS_RETURN_OK == FS_GetNextFileInfo(&fs_drive, &auto_file_attr))
                        {
                            if (fs_drive != unit_id)
                            {
                                break;
                            }
                            if (auto_file_attr.file_type == FS_TYPE_OPCODE_FILE_TYPE)
                            {
                            #if (FS_TYPE_MAX_NUM_OF_FILE_OP_CODE == 1)
                                /* One image file system
                                 */
                                if (TRUE == auto_file_attr.startup_file)
                                {
                                    FS_ResetStartupFilename(fs_drive, FS_TYPE_OPCODE_FILE_TYPE);

                                    if (FS_RETURN_OK != FS_DeleteFile(unit_id, auto_file_attr.file_name))
                                    {
                                        printf("Failed to delete non-statup file %s\r\n", auto_file_attr.file_name);
                                        return;
                                    }
                                }
                            #elif (FS_TYPE_MAX_NUM_OF_FILE_OP_CODE == 2)
                                /* Two opcodes file system
                                 */
                                if (FALSE == auto_file_attr.startup_file)
                                {
                                    if (FS_RETURN_OK != FS_DeleteFile(unit_id, auto_file_attr.file_name))
                                    {
                                        printf("Failed to delete non-statup file %s\r\n", auto_file_attr.file_name);
                                        return;
                                    }
                                }

                                /* If the startup image filename is already op1.bix, the new upgrade file shall
                                 * be saved as op2.bix
                                 */
                                if (TRUE == auto_file_attr.startup_file)
                                {
                                    char tmp_file_name[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1];

                                    strncpy(tmp_file_name, (char *)auto_file_attr.file_name, sizeof(tmp_file_name) - 1);
                                    tmp_file_name[sizeof(tmp_file_name) - 1] = '\0';
                                    L_STDLIB_StringN_To_Lower((UI8_T *)tmp_file_name, strlen(tmp_file_name));

                                    if (0 == strcmp(tmp_file_name, XFER_MGR_AUTO_UPGRADE_OPCODE_DEFAULT_FILENAME))
                                    {
                                        strcpy((char *)mtext->destfile, XFER_MGR_AUTO_UPGRADE_OPCODE_SECOND_FILENAME);
                                    }
                                }
                            #else
                                /* NO IMPLEMENT IF FS_TYPE_MAX_NUM_OF_FILE_OP_CODE OVER 2 (can't test ...)
                                 *
                                 * the rule for new file name for over 3 file system SHALL be
                                 * op1.bix -> op2.bix -> op3.bix ->...->opn.bix -> op1.bix
                                 *
                                 * e.g., on an allow 3 runtime file system
                                 *       the new file name be op1.bix -> op2.bix -> op3.bix -> op1.bix
                                 */
                                # error "Xfer: No support the FS_TYPE_MAX_NUM_OF_FILE_OP_CODE over 2"
                            #endif
                            }
                        }

                    }/* end if (TRUE == mtext->auto_assign_dest_runtime_filename)*/
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */
                }


                if ((FS_RETURN_OK == FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
                    && (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile))
                   )
                {
                    /* save original source filename cause it will be change when sync to slave
                     */
                    strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                    original_filename[sizeof(original_filename)-1] = '\0';

                    /* write new opcode to "master + slaves"
                     */
                    mtext->unit_id = unit_id;
                    strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                    mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                    auto_download_sync_to_master = TRUE;
                    XFER_MGR_AutoDownLoad_Syn (mtext);
                    auto_download_sync_to_master = FALSE;

                    /* revert to original source filename
                     */
                    strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                    mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';

#if (SYS_CPNT_ONIE_SUPPORT == TRUE)
                    if (FS_RETURN_OK !=FS_SetStartupFilename(unit_id, mtext->file_type, mtext->destfile))
                    {
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                        TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                        break;
                    }
#endif

                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                    TftpMgrEntry.tftpStatus = XFER_MGR_SUCCESS;
                }
                else
                {

#if (SYS_CPNT_SINGLE_RUNTIME_IMAGE == TRUE)

                    /* For projects that have different max number of runtime image on different
                     * board ids(For example, ES3510MA-FLF-38), need to check the real runtime
                     * partition number to determine
                     * whether the code snippet shown below should be run
                     */
                    if(FS_GetNumOfPartitionByTypeFromPartitionTable(TYPE_RUNTIME)==1)
                    {
                        /* For single image case: Use new opcode name as startup opcode name.
                         * Before writing new opcode, to get startup opcode and then delete it.
                         */
                        if (    (FS_TYPE_OPCODE_FILE_TYPE == mtext->file_type)
                            &&  (FS_RETURN_OK == FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
                           )
                        {
                            /* 1. Check available free space.
                             * 2. Reset fs startup table.
                             * 3. Delete current startup opcode.
                             */
                            if (    (FALSE == XFER_MGR_CheckFreeSizeByFileType(FS_TYPE_OPCODE_FILE_TYPE, unit_id, startup_file_name, info.tftp_download_length))
                                ||  (FS_RETURN_OK != FS_DeleteFile(unit_id, startup_file_name))
                               )
                            {
                                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                                TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                                break;
                            }
                            /* 4. Set startup opcode */
                            mtext->is_next_boot = TRUE;
                        }
                    }

#endif /* End of #if (SYS_CPNT_SINGLE_RUNTIME_IMAGE == TRUE) */

                    /* write new opcode to "master only" */
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);

                    ret = FS_WriteFile(  unit_id, mtext->destfile,
                                         (UI8_T *)"Xfer",
                                         mtext->file_type,
                                         mtext->buf,
                                         info.tftp_download_length ,0);

                    if(FS_RETURN_OK == ret)
                    {
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);

                        if(TRUE == mtext->is_next_boot &&
                           FS_RETURN_OK != FS_SetStartupFilename(unit_id, FS_TYPE_OPCODE_FILE_TYPE, mtext->destfile))
                        {
                            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SET_STARTUP_ERROR);
                            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SET_STARTUP_ERROR;
                            TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                        }
                        else
                        {
                            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                            TftpMgrEntry.tftpStatus = XFER_MGR_SUCCESS;
                        }
                    }
                    else
                    {
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                        TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                    } /* End of if (FS_WriteFile(...)) */
                }

                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
            }
            else if(mtext->file_type == FS_FILE_TYPE_CONFIG)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);

                ret = FS_WriteFile( unit_id, mtext->destfile,
                                    (UI8_T *)"Xfer",
                                    mtext->file_type,
                                    mtext->buf,
                                    info.tftp_download_length,0);

                if(ret  == FS_RETURN_OK)
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
                    if(TRUE == mtext->is_next_boot &&
                       FS_RETURN_OK != FS_SetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, mtext->destfile))
                    {
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SET_STARTUP_ERROR);
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SET_STARTUP_ERROR;
                        TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                    }
                    else
                    {
                        /* if the destination file is current startup file then sync to slave
                         */
                        if (    (FS_RETURN_OK == FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
                            &&  (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile))
                           )
                        {
                            /* save original source filename cause it will be change when sync to slave
                             */
                            strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                            original_filename[sizeof(original_filename)-1] = '\0';

                            /* sync to slaves
                             */
                            mtext->unit_id = unit_id;
                            strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                            mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                            XFER_MGR_AutoDownLoad_Syn(mtext);

                            /* revert to original source filename
                             */
                            strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                            mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                        }

                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                        TftpMgrEntry.tftpStatus = XFER_MGR_SUCCESS;
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                    }

                    TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
#if(SYS_CPNT_XFER_RESTORE_CFG_RESTART == TRUE && SYS_CPNT_DBSYNC_TXT == TRUE)
                    DBSYNC_TXT_MGR_SetDirty(FALSE);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_START_REBOOTING);
                    STKCTRL_PMGR_ReloadSystem();
#endif /* #if(SYS_CPNT_XFER_RESTORE_CFG_RESTART == TRUE) */
                }
                else
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                    TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                }/*End of if(FS_WriteFile(...))*/
            }
            else if (mtext->file_type == FS_FILE_TYPE_LICENSE)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);

                ret = FS_WriteFile( unit_id, mtext->destfile,
                                    (UI8_T *)"Xfer",
                                    mtext->file_type,
                                    mtext->buf,
                                    info.tftp_download_length,0);

                if(ret == FS_RETURN_OK)
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                    TftpMgrEntry.tftpStatus = XFER_MGR_SUCCESS;
                    TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                }
                else
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                    TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                }/*End of if(FS_WriteFile(...))*/

            }
#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
            else if (mtext->file_type == FS_FILE_TYPE_TOTAL)
            {

                if (FALSE == XFER_MGR_CheckLoaderImageValidation(mtext))
                {
                    break;
                }

                /* write new loader */
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_LOADER_TO_FLASH);
                ret = FS_WriteLoader(unit_id, mtext->buf+sizeof(LDR_INFO_BLOCK_T), info.tftp_download_length-sizeof(LDR_INFO_BLOCK_T));

                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
                if(FS_RETURN_OK == ret)
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                    TftpMgrEntry.tftpStatus    = XFER_MGR_SUCCESS;
                }
                else
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                    TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                } /* End of if (FS_WriteLoader(...)) */

                TftpMgrEntry.tftpAction    = VAL_tftpAction_notDownloading;
            }
#endif  /* SYS_CPNT_XFER_DOWNLOAD_LOADER */
            break;

        case XFER_MGR_LOCAL_TO_REMOTE:
        case XFER_MGR_STARTUP_TO_REMOTE:
            if(FS_ReadFile( unit_id, mtext->srcfile,  mtext->buf,
                            SYS_ADPT_MAX_FILE_SIZE,
                            &send_text->uploadLength)!= FS_RETURN_OK)
            {
                XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                break;
            }
            mtext->length=send_text->uploadLength;

            strcpy((char *)send_text->filename, (char *)mtext->destfile);
            XFER_DNLD_tftp(&send_msg);
            XFER_MGR_WaitForTransmitCompleted();

            if ( info.tftp_status != XFER_DNLD_TFTP_SUCCESS)
            {
                XFER_MGR_CallBack(mtext, info.tftp_status);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                file_copy_mgt_entry.status = info.tftp_status;
                break;
            }

            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
            TftpMgrEntry.tftpStatus = XFER_MGR_SUCCESS;
            TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
            break;

        case XFER_MGR_REMOTE_TO_PUBLICKEY:
        {
            SSHD_SetUserPublicKeyResult_T result;

            send_text->uploadLength = USER_DSA_PUBLIC_KEY_FILE_LENGTH;
            strcpy((char *)send_text->filename, (char *)mtext->srcfile);
            XFER_DNLD_tftp(&send_msg);
            XFER_MGR_WaitForTransmitCompleted();

            if (info.tftp_status != XFER_DNLD_TFTP_SUCCESS)
            {
                XFER_MGR_CallBack(mtext, info.tftp_status);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
                file_copy_mgt_entry.status = info.tftp_status;
                break;
            }

            SSHD_PMGR_CopyUserPublicKey(mtext->buf);

            result = SSHD_PMGR_SetUserPublicKey(
                (UI8_T *)mtext->publickey_username, mtext->publickey_type);

            if (SSHD_SET_USER_PUBLIC_KEY_SUCC == result)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
                TftpMgrEntry.tftpStatus = XFER_MGR_SUCCESS;
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
            }
            else
            {
                switch (result)
                {
                    case SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY:
                    case SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY_TYPE:
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                        break;

                    case SSHD_SET_USER_PUBLIC_KEY_STORAGE_FULL:
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_STORAGE_FULL;
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_STORAGE_FULL);
                        break;

                    case SSHD_SET_USER_PUBLIC_KEY_WRITE_ERROR:
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                        break;

                    case SSHD_SET_USER_PUBLIC_KEY_FAIL:
                    case SSHD_SET_USER_PUBLIC_KEY_OUT_OF_MEMORY:
                    default:
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR;
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR);
                        break;
                }

                TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            }

            break;
        }

        default:
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_ERROR;
            break;
    } /* End of switch */

    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    mtext->status = file_copy_mgt_entry.status;
    XFER_MGR_SendFileCopyTrap(mtext);
    is_file_copy_mgt_used = FALSE;
    BUFFER_MGR_Free(mtext->buf);
    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
    L_MM_Free(send_msg.mtext);
    return ;
}/* End of XFER_MGR_CopyFile_Syn() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_RemoteToStream_Syn
 *------------------------------------------------------------------------
 * FUNCTION: The function is remote file copy to loacl the memory stream
 * INPUT   : Mtext_T *mtext
 * OUTPUT  :
 * RETURN  : none
 * NOTE    : from tftp server download file to buf
 *------------------------------------------------------------------------*/
static void  XFER_MGR_RemoteToStream_Syn (Mtext_T *mtext)
{
    XFER_DNLD_Msg_T     send_msg;
    XFER_DNLD_Mtext_T   *send_text;

    /*BODY
    */
    if((send_text = L_MM_Malloc(sizeof(XFER_DNLD_Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_REMOTETOSTREAM_SYN)) ) == NULL)
    {
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        mtext->status = file_copy_mgt_entry.status;
        XFER_MGR_SendFileCopyTrap(mtext);
        is_file_copy_mgt_used = FALSE;
        BUFFER_MGR_Free(mtext->buf);
        printf("XFER_MGR_RemoteToStream_Syn : L_MM_Malloc FAIL!!\n");
        return;
    }

    send_msg.is_download =     XFER_MGR_REMOTE_TO_LOCAL;
    send_text->tftp_retry_times = XFER_OM_GetTftpRetryTimes();
    send_text->tftp_timeout     = XFER_OM_GetTftpTimeout();
    memcpy( &(send_text->server_ip), &(mtext->server_ip), sizeof(send_text->server_ip));
    send_text->buf =            mtext->buf;
    send_text->uploadLength =   mtext->length;
    strcpy((char *)send_text->filename, (char *)mtext->srcfile);
    send_msg.server_type =      mtext->server_type;
    send_text->action = XFER_DNLD_ACTION_GET_WHOLE_FILE;

    XFER_MGR_CopyUserInfoByServerType(mtext->server_type, mtext->username, mtext->password, send_text->username, send_text->password);

    send_msg.mtext = send_text;

    XFER_DNLD_tftp(&send_msg);
    XFER_MGR_WaitForTransmitCompleted();

    if ( info.tftp_status != XFER_DNLD_TFTP_SUCCESS)
    {
        XFER_MGR_CallBack(mtext, info.tftp_status);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        file_copy_mgt_entry.status = info.tftp_status;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        mtext->status = file_copy_mgt_entry.status;
        XFER_MGR_SendFileCopyTrap(mtext);
        is_file_copy_mgt_used = FALSE;
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free ( send_msg.mtext);
        return ;
    }

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
    if (TRUE == info.is_partial_provision)
    {
        file_copy_mgt_entry.ipc_message_q = mtext->ipc_message_q;
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_BUSY;
        file_copy_mgt_entry.cookie = mtext->cookie;
        file_copy_mgt_entry.callback = mtext->callback;
        file_copy_mgt_entry.buf = mtext->buf;

        if (strlen((char *)mtext->buf) > SYS_ADPT_MAX_FILE_SIZE)
        {
            mtext->buf[SYS_ADPT_MAX_FILE_SIZE-1] = '\0';
        }

        L_MM_Free(send_msg.mtext);
        CLI_PMGR_PartialProvision(mtext->buf);
        return;
    }
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

    /*snmp mib support copy tftp public-key*/
	if ((mtext->file_type == FS_FILE_TYPE_CERTIFICATE)
                && (USER_DSA_PUBLIC_KEY_FILE_LENGTH == mtext->length))
    {
        if(is_file_copy_mgt_used)
        {
            SSHD_SetUserPublicKeyResult_T result;

            SSHD_PMGR_CopyUserPublicKey(mtext->buf);

            result = SSHD_PMGR_SetUserPublicKey(
                (UI8_T *)file_copy_mgt_entry.publickey_username,
                file_copy_mgt_entry.publickey);
            if (SSHD_SET_USER_PUBLIC_KEY_SUCC != result)
            {
                switch (result)
                {
                    case SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY:
                    case SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY_TYPE:
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
                        break;

                    case SSHD_SET_USER_PUBLIC_KEY_STORAGE_FULL:
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_STORAGE_FULL;
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_STORAGE_FULL);
                        break;

                    case SSHD_SET_USER_PUBLIC_KEY_WRITE_ERROR:
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                        break;

                    case SSHD_SET_USER_PUBLIC_KEY_FAIL:
                    case SSHD_SET_USER_PUBLIC_KEY_OUT_OF_MEMORY:
                    default:
                        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_ERROR;
                        break;
                }

                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
                TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
                file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            }
            BUFFER_MGR_Free(mtext->buf);
        }
    }

    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
    TftpMgrEntry.tftpStatus = XFER_MGR_SUCCESS;
    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    mtext->status = file_copy_mgt_entry.status;
    XFER_MGR_SendFileCopyTrap(mtext);
    is_file_copy_mgt_used = FALSE;
    L_MM_Free ( send_msg.mtext);

    if (info.is_end_with_provision)
    {
        CLI_PMGR_Notify_EnterTransitionMode(mtext->buf-strlen(CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD)); /* notify to enter transition mode */
    }
    return ;
}/* End of XFER_MGR_RemoteToStream_Syn() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_StreamToRemote_Syn
 *------------------------------------------------------------------------
 * FUNCTION: The function is loacl the memory stream  copy to remote file
 * INPUT   : Mtext_T *mtext
 * OUTPUT  :
 * RETURN  : none
 * NOTE    : tftp upload
 *------------------------------------------------------------------------*/
static void  XFER_MGR_StreamToRemote_Syn (Mtext_T *mtext)
{
    XFER_DNLD_Msg_T     send_msg;
    XFER_DNLD_Mtext_T   *send_text;

    /* BODY
     */
     /*if mtext->length is 0 , it's means running config not put in  mtext ->buf , need collect it */
    if((mtext->file_type == FS_FILE_TYPE_CONFIG) && (mtext->length == 0))
    {
        /*collect running-config*/
        if (CLI_MGR_RUNCFG_RETURN_NO_ENOUGH_MEMORY == CLI_PMGR_Get_RunningCfg(mtext->buf, SYS_ADPT_MAX_FILE_SIZE))
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            mtext->status = file_copy_mgt_entry.status;
            XFER_MGR_SendFileCopyTrap(mtext);
            is_file_copy_mgt_used = FALSE;
            BUFFER_MGR_Free(mtext->buf);
            /* 2007.05.22 aken, execute the BUFFER_MGR_Free() before XFER_MGR_CallBack()*/
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            return ;
        }
		mtext->length = strlen((char *)mtext->buf);
    }
	else
	{
		    if(strcmp((char *)mtext->destfile, SYS_DFLT_restartConfigFile)!=0)
                mtext->buf =(UI8_T *) BUFFER_MGR_GetPtr(mtext->offset);
	}


    if((send_text = L_MM_Malloc(sizeof(XFER_DNLD_Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_STREAMTOREMOTE_SYN)) )== NULL)
    {
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        printf("XFER_MGR_StreamToRemote_Syn : L_MM_Malloc FAIL!!\n");
        mtext->status = file_copy_mgt_entry.status;
        XFER_MGR_SendFileCopyTrap(mtext);
        is_file_copy_mgt_used = FALSE;
        BUFFER_MGR_Free(mtext->buf);
        return;
    }

    send_msg.is_download =     XFER_MGR_LOCAL_TO_REMOTE;
    send_text->tftp_retry_times = XFER_OM_GetTftpRetryTimes();
    send_text->tftp_timeout     = XFER_OM_GetTftpTimeout();
    memcpy(&(send_text->server_ip), &(mtext->server_ip), sizeof(send_text->server_ip));
    send_text->buf =           mtext->buf;
    send_text->uploadLength =  mtext->length;
    strcpy ((char *)send_text->filename, (char *)mtext->destfile);

    send_msg.server_type =     mtext->server_type;
    send_text->action = XFER_DNLD_ACTION_GET_WHOLE_FILE;

    XFER_MGR_CopyUserInfoByServerType(mtext->server_type, mtext->username, mtext->password, send_text->username, send_text->password);

    send_msg.mtext = send_text;

    XFER_DNLD_tftp(&send_msg);
    XFER_MGR_WaitForTransmitCompleted();

    if ( info.tftp_status != XFER_DNLD_TFTP_SUCCESS)
    {
        XFER_MGR_CallBack(mtext, info.tftp_status);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        file_copy_mgt_entry.status = info.tftp_status;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        mtext->status = file_copy_mgt_entry.status;
        XFER_MGR_SendFileCopyTrap(mtext);
        is_file_copy_mgt_used = FALSE;
        L_MM_Free ( send_msg.mtext);
        BUFFER_MGR_Free(mtext->buf);
        return ;
    }

    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    mtext->status = file_copy_mgt_entry.status;
    XFER_MGR_SendFileCopyTrap(mtext);
    is_file_copy_mgt_used = FALSE;
    L_MM_Free ( send_msg.mtext);
    BUFFER_MGR_Free(mtext->buf);
    return ;
}/* End of XFER_MGR_RemoteToStream_Syn() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_StreamToLocal_Syn
 *------------------------------------------------------------------------
 * FUNCTION: This function will save memory stream to local file
 * INPUT   : UI8_T *filename
 *           UI8_T *xbuffer
 *           UI32_T file_type
 *           UI32_T stream_length
 * OUTPUT  : None
 * RETURN  :
 * NOTE    :
 *-----------------------------------------------------------------*/
static void XFER_MGR_StreamToLocal_Syn (Mtext_T *mtext)
{
    UI32_T    ret;
    UI32_T    unit_id;
    UI8_T     startup_file_name[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME + 1];
    char      original_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME + 1] = {0};

    if (XFER_MGR_IsFileNumExceededByType(mtext->destfile, mtext->file_type) == TRUE)
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_FILE_NUM_EXCEED);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_FILE_NUM_EXCEED;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        XFER_MGR_SendFileCopyTrap(mtext);
        is_file_copy_mgt_used = FALSE;
        BUFFER_MGR_Free(mtext->buf);
        /* 2007.05.22 aken, execute the BUFFER_MGR_Free() before XFER_MGR_CallBack()*/
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        return ;
    }

    /* BODY
     */
    /*if mtext->length is 0 , it's means running config not put in  mtext ->buf , need collect it */
    if((mtext->file_type == FS_FILE_TYPE_CONFIG) && (mtext->length == 0))
    {
        /*collect running-config*/
        if (CLI_MGR_RUNCFG_RETURN_NO_ENOUGH_MEMORY == CLI_PMGR_Get_RunningCfg(mtext->buf, SYS_ADPT_MAX_FILE_SIZE))
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            mtext->status = file_copy_mgt_entry.status;
            XFER_MGR_SendFileCopyTrap(mtext);
            is_file_copy_mgt_used = FALSE;
            BUFFER_MGR_Free(mtext->buf);
            /* 2007.05.22 aken, execute the BUFFER_MGR_Free() before XFER_MGR_CallBack()*/
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            return ;
        }
		mtext->length = strlen((char *)mtext->buf);
    }
	else
	{
        if(strcmp((char *)mtext->destfile, SYS_DFLT_restartConfigFile)!=0)
            mtext->buf =(UI8_T *) BUFFER_MGR_GetPtr(mtext->offset);
	}

    STKTPLG_POM_GetMyUnitID(&unit_id);
    XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);
    ret = FS_WriteFile(unit_id,mtext->destfile, (UI8_T *)"Xfer", mtext->file_type, mtext->buf, mtext->length,0);

    if (ret == FS_RETURN_OK)
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
        if(TRUE == mtext->is_next_boot &&
           FS_RETURN_OK != FS_SetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, mtext->destfile))
        {

            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SET_STARTUP_ERROR);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SET_STARTUP_ERROR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        }
        else
        {
            /* if the destination file is current startup file then sync to slave
             */
            if (    (FS_RETURN_OK == FS_GetStartupFilename(unit_id, FS_FILE_TYPE_CONFIG, startup_file_name))
                &&  (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile))
               )
            {
                /* save original source filename cause it will be change when sync to slave
                 */
                strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                original_filename[sizeof(original_filename)-1] = '\0';

                /* sync to slaves
                 */
                mtext->unit_id = unit_id;
                strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                XFER_MGR_AutoDownLoad_Syn(mtext);

                /* revert to original source filename
                 */
                strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
            }

            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        }
        /* 2007.05.22 aken, execute the BUFFER_MGR_Free() before XFER_MGR_CallBack()*/
        /* XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED); */
    }
    else
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        /* 2007.05.22 aken, execute the BUFFER_MGR_Free() before XFER_MGR_CallBack()*/
        /* XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED); */
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    }

    mtext->status = file_copy_mgt_entry.status;
    XFER_MGR_SendFileCopyTrap(mtext);
    is_file_copy_mgt_used = FALSE;
    BUFFER_MGR_Free(mtext->buf);
    /* 2007.05.22 aken, execute the BUFFER_MGR_Free() before XFER_MGR_CallBack()*/
    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
    return ;
}/* End of XFER_MGR_StreamToLocal_Syn() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_UnitToLocal_Syn
 *------------------------------------------------------------------------
 * FUNCTION: The function copy file from unit to loacl
 * INPUT   : XFER_MGR_Msg_T *mtext
 * OUTPUT  :
 * RETURN  : none
 * NOTE    : Must free mtext
 *------------------------------------------------------------------------*/
static void XFER_MGR_UnitToLocal_Syn (Mtext_T *mtext)
{
    UI32_T              read_count;
    UI32_T              unit_id;

    STKTPLG_POM_GetMyUnitID(&unit_id);

    if(FS_ReadFile( mtext->unit_id, mtext->srcfile,  mtext->buf, SYS_ADPT_MAX_FILE_SIZE, &read_count) != FS_RETURN_OK)
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    }
    else
    {
        mtext->length=read_count;
        XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);
        if(FS_WriteFile (unit_id, mtext->destfile, (UI8_T *)"Xfer", mtext->file_type, mtext->buf, read_count,0) == FS_RETURN_OK )
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        }
        else
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        }
    }

    mtext->status = file_copy_mgt_entry.status;
    XFER_MGR_SendFileCopyTrap(mtext);
    is_file_copy_mgt_used = FALSE;
    BUFFER_MGR_Free(mtext->buf);
    return ;
}/* End of XFER_MGR_UnitToLocal_Syn() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_LocalToUnit_Syn
 *------------------------------------------------------------------------
 * FUNCTION: The function copy file from local to unit
 * INPUT   : XFER_MGR_Msg_T *mtext
 * OUTPUT  :
 * RETURN  : none
 * NOTE    : Must free mtext
 *------------------------------------------------------------------------*/
static void XFER_MGR_LocalToUnit_Syn (Mtext_T *mtext)
{
    UI32_T              read_count;
    UI32_T              unit_id;

    STKTPLG_POM_GetMyUnitID(&unit_id);

    if(FS_ReadFile( unit_id, mtext->srcfile,  mtext->buf, SYS_ADPT_MAX_FILE_SIZE, &read_count) != FS_RETURN_OK)
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    }
    else
    {
        mtext->length=read_count;
        XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);
        if(FS_WriteFile (mtext->unit_id, mtext->destfile, (UI8_T *)"Xfer", mtext->file_type, mtext->buf, read_count,0) == FS_RETURN_OK )
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        }
        else
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        }
    }

    mtext->status = file_copy_mgt_entry.status;
    XFER_MGR_SendFileCopyTrap(mtext);
    is_file_copy_mgt_used = FALSE;
    BUFFER_MGR_Free(mtext->buf);
    return ;
}/* End of XFER_MGR_UnitToLocal_Syn() */

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_GetRemoteImageSWVersion
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves the software version from a remote image
 * INPUT   : server_type - XFER_MGR_REMOTE_SERVER_FTP, from ftp server
 *                         XFER_MGR_REMOTE_SERVER_TFTP, from tftp server
 *           server_ip_p - server address
 *           username    - (option) login username
 *           password    - (option) login password
 *           filepath    - image filepath. E.g., /loc/op1.bix
 * OUTPUT  : version     - image software version field (network order)
 * RETURN  : TRUE (success) / FALSE (fail)
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_GetRemoteImageSWVersion(UI32_T server_type,
                                               L_INET_AddrIp_T *server_ip_p,
                                               const UI8_T *username,
                                               const UI8_T *password,
                                               const UI8_T *filepath,
                                               UI8_T *version)
{
#if (SYS_CPNT_ONIE_SUPPORT==FALSE)
    image_header_t      *imghdr;
#endif
    XFER_DNLD_Mtext_T   *send_text;
    XFER_DNLD_Msg_T     send_msg;
    Mtext_T             mtext;
    UI32_T              img_header_size;

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    img_header_size=AOS_ONIE_INSTALLER_MAX_IMG_HEADER_SIZE;
#else
    img_header_size=sizeof(image_header_t);
#endif


    if (NULL == version || NULL == filepath)
    {
        return FALSE;
    }

#if (SYS_CPNT_XFER_FTP == TRUE)
    /* Login username and password for FTP are required
     */
    if (XFER_MGR_REMOTE_SERVER_FTP == server_type)
    {
        if (NULL== username || NULL == password)
        {
            return FALSE;
        }
    }
#endif

    if((send_text = L_MM_Malloc(sizeof(XFER_DNLD_Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_AUTOUPGRADE)) )== NULL)
    {
        if (TRUE == debug_flag)
        {
            printf("XFER_MGR_GetRemoteImageSWVersion : Failed to alloc memory to save the message context\r\n");
        }

        printf("Internal error\r\n");
        return FALSE;
    }

    if((send_text->buf = L_MM_Malloc(img_header_size, L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_AUTOUPGRADE)) )== NULL)
    {
        if (TRUE == debug_flag)
        {
            printf("XFER_MGR_GetRemoteImageSWVersion : Failed to alloc memory to save the image header\r\n");
        }

        printf("Internal error\r\n");
        L_MM_Free(send_text);
        return FALSE;
    }
    send_text->uploadLength = img_header_size;
    send_text->action       = XFER_DNLD_ACTION_GET_PIECE_OF_FILE;
    memcpy(&(send_text->server_ip), server_ip_p, sizeof(send_text->server_ip));

#if (SYS_CPNT_XFER_FTP == TRUE)
    if (XFER_MGR_REMOTE_SERVER_FTP == server_type)
    {
        strncpy((char *)send_text->username, (char *)username, MAXSIZE_fileCopyServerUserName);
        send_text->username[MAXSIZE_fileCopyServerUserName] = '\0';
        strncpy((char *)send_text->password, (char *)password, MAXSIZE_fileCopyServerPassword);
        send_text->password[MAXSIZE_fileCopyServerPassword] = '\0';
    }
#endif
    strncpy((char *)send_text->filename, (char *)filepath, XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME);
    send_text->filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME] = '\0';

    send_msg.is_download        = TRUE;
    send_text->tftp_retry_times = XFER_OM_GetTftpRetryTimes();
    send_text->tftp_timeout     = XFER_OM_GetTftpTimeout();
    send_msg.server_type        = server_type;
    send_msg.mtext              = send_text;

    /* Reset the context of info
     */
    memset(&info, 0, sizeof(info));
    XFER_DNLD_tftp(&send_msg);
    XFER_MGR_WaitForTransmitCompleted();

    if ( info.tftp_status != XFER_DNLD_TFTP_SUCCESS)
    {
        if (TRUE == debug_flag)
        {
            printf("XFER_MGR_GetRemoteImageSWVersion : Failed to download image header\r\n");
        }

        L_MM_Free(send_text->buf);
        L_MM_Free(send_text);
        return FALSE;
    }

    /* Check download opcode header
     */
    memset(&mtext, 0, sizeof(mtext));
    mtext.buf = send_text->buf;

    /* Read image header and image length
     */
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    if(XFER_MGR_GetSWVersionFromONIEInstallerStream(mtext.buf, version)==FALSE)
    {
        L_MM_Free(send_text->buf);
        L_MM_Free(send_text);
        return FALSE;
    }
#else
    imghdr = (image_header_t *)mtext.buf;
    memcpy(version, imghdr->ih_ver, SYS_ADPT_FW_VER_STR_LEN+1);
#endif

    L_MM_Free(send_text->buf);
    L_MM_Free(send_text);
    return TRUE;
} /* End XFER_MGR_GetRemoteImageSWVersion() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_WaitForRoutingInterfaceReady
 *------------------------------------------------------------------------
 * FUNCTION: Wait routing interface ready with timeout
 * INPUT   : ipaddr     - server IP address
 *           timeout    - waiting timeout in second
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void XFER_MGR_WaitForRoutingInterfaceReady(const L_INET_AddrIp_T *ipaddr, UI32_T timeout)
{
    L_INET_AddrIp_T src_ip;
    L_INET_AddrIp_T dst_ip;
    L_INET_AddrIp_T nexthop_ip;
    UI32_T out_ifindex;
    UI32_T expire_ticks;

    SYS_TIME_GetSystemUpTimeByTick(&expire_ticks);
    expire_ticks += timeout * SYS_BLD_TICKS_PER_SECOND;

    memset(&dst_ip, 0x0, sizeof(dst_ip));
    memset(&src_ip, 0x0, sizeof(src_ip));

    memcpy(&dst_ip, ipaddr, sizeof(dst_ip));

    while (IPAL_RESULT_OK != IPAL_ROUTE_RouteLookup(&dst_ip, &src_ip, &nexthop_ip, &out_ifindex))
    {
        UI32_T ticks;

        SYS_TIME_GetSystemUpTimeByTick(&ticks);

        if (expire_ticks < ticks)
        {
            break;
        }

        SYSFUN_Sleep(20);
    }
}



/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_UpgradeFile
 *------------------------------------------------------------------------
 * FUNCTION: Upgrades image if detected a new version image
 * INPUT   : immediately    -  do file operation immediately and not wait other
 *                             file operation
 *                             TRUE, if XFER is busy, return FAIL and no enqueue
 *                             If XFER is available, return TRUE and enqueue
 *                             FALSE, enqueue and return TRUE.
 * OUTPUT  : None
 * RETURN  : TRUE: succes; FALSE: fail
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_UpgradeFile(BOOL_T immediately)
{
    Mtext_T    *mtext;
    XFER_Msg_T msg;
    BOOL_T     set_is_busy_flag = FALSE; /* use to clear info.is_busy flag if
                                       * it be set by this API */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* The check for Auto Upgrade will be done only once on detected a valid IP address.
     */
    if (FALSE == xfer_mgr_auto_upgrade_check)
    {
        return TRUE;
    }

    xfer_mgr_auto_upgrade_check = FALSE;

    if (VAL_fileAutoUpgradeOpCodeStatus_enabled != XFER_OM_GetAutoOpCodeUpgradeStatus())
    {
        return TRUE;
    }

    if ( !XFER_MGR_Transaction_Begin(immediately, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_AUTOUPGRADE_FUNC_NO,
            EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if ((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_AUTOUPGRADE)) )== NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_AUTOUPGRADE_FUNC_NO,
            EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    memset(mtext, 0, sizeof(Mtext_T));

    if ((mtext->buf = BUFFER_MGR_Allocate()) == 0)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        L_MM_Free (mtext);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_AUTOUPGRADE_FUNC_NO,
            EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    mtext->ipc_message_q = SYS_BLD_XFER_GROUP_IPCMSGQ_KEY;
    mtext->mode      = XFER_MGR_REMOTE_TO_LOCAL;
    mtext->file_type = FS_TYPE_OPCODE_FILE_TYPE;
    mtext->cookie    = 0;
    mtext->callback  = NULL;
    mtext->length    = 0;

    msg.mtype        = XFER_MGR_UPGRADEFILE;
    msg.mtext        = mtext;

    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
    req_msg_p->msg_type = 1;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf, &msg, sizeof(XFER_Msg_T));

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

}/* End of XFER_MGR_UpgradeFile() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_UpgradeFile_Syn
 *------------------------------------------------------------------------
 * FUNCTION: Upgrades image if detected a new version image
 * INPUT   : mtext    - Xfer message context pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static void XFER_MGR_UpgradeFile_Syn (Mtext_T *mtext)
{
    UI32_T  unit_id;
    UI32_T  timeout; /* timeout value in seconds for waiting routing interface ready */
    UI8_T   old_mainboard_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};
    UI8_T   new_mainboard_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};
    char    *list_a[ XFER_MGR_URL_TOKEN_MAX ];
    char    *file_loc_p;

    STKTPLG_POM_GetMyUnitID(&unit_id);

    file_loc_p = malloc( MAXSIZE_fileAutoUpgradeOpCodePath + 1 );

    if (NULL == file_loc_p)
    {
        if (TRUE == debug_flag)
        {
            printf("XFER_MGR_UpgradeFile_Syn : Failed to alloc memory to save the copy of the opcode-dir-url\r\n");
        }
        BUFFER_MGR_Free(mtext->buf);
        return;
    }

    XFER_OM_GetAutoOpCodeUpgradePath(file_loc_p);
    /* Nothing to do, if the opcode-dir-url is empty.
     */
    if ('\0' == file_loc_p[0])
    {
        if (TRUE == debug_flag)
        {
            printf("XFER_MGR_UpgradeFile_Syn : Opcode-dir-url is empty\r\n");
        }
        BUFFER_MGR_Free(mtext->buf);
        free(file_loc_p);
        return;
    }

    if (FALSE == XFER_MGR_ParseUrl(file_loc_p, list_a))
    {
        if (TRUE == debug_flag)
        {
            printf("XFER_MGR_UpgradeFile_Syn : Failed to parse opcode-dir-url\r\n");
        }
        BUFFER_MGR_Free(mtext->buf);
        free(file_loc_p);
        return;
    }

    printf("\r\n");
    printf("Automatic Upgrade is looking for a new image\r\n");

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       list_a[XFER_MGR_URL_TOKEN_HOST],
                                                       (L_INET_Addr_T *)&mtext->server_ip,
                                                       sizeof(mtext->server_ip)))
    {
        if (TRUE == debug_flag)
        {
            printf("%s is not valid IP address\r\n", list_a[XFER_MGR_URL_TOKEN_HOST]);
        }
        BUFFER_MGR_Free(mtext->buf);
        free(file_loc_p);
        return;
    }

    /* transfer file via ftp or tftp
     */
    L_STDLIB_StringN_To_Lower((UI8_T *)list_a[XFER_MGR_URL_TOKEN_SCHEME], strlen(list_a[XFER_MGR_URL_TOKEN_SCHEME]));

    if (0 == strcmp(list_a[XFER_MGR_URL_TOKEN_SCHEME], "tftp"))
    {
        mtext->server_type = XFER_MGR_REMOTE_SERVER_TFTP;
    }
#if (SYS_CPNT_XFER_FTP == TRUE)
    else
    {
        mtext->server_type = XFER_MGR_REMOTE_SERVER_FTP;
        strcpy((char *)mtext->username, (list_a[XFER_MGR_URL_TOKEN_USER]!=NULL)?list_a[XFER_MGR_URL_TOKEN_USER]:XFER_MGR_DFLT_USERNAME);
        strcpy((char *)mtext->password, (list_a[XFER_MGR_URL_TOKEN_PASSWORD]!=NULL)?list_a[XFER_MGR_URL_TOKEN_PASSWORD]:XFER_MGR_DFLT_PASSWORD);
    }
#endif

    mtext->srcfile[0] = '\0';

    if (NULL != list_a[XFER_MGR_URL_TOKEN_PATHNAME])
    {
        /* Starting search from root directory
         */
        strcat ((char *) mtext->srcfile, "/");
        strcat ((char *) mtext->srcfile, list_a[XFER_MGR_URL_TOKEN_PATHNAME]);
    }


    {
        char filename[MAXSIZE_fileAutoUpgradeOpCodeFileName + 1];

        XFER_MGR_GetAutoOpCodeUpgradeFileName(filename);

        strcat ((char *)mtext->srcfile, filename);
    }


    /* Wait for routing interface ready.
     * When the server is in different sub-net. Before send any packet, it need
     * to wait the routing inferface is ready.
     */
    timeout = 5; /* seconds */
    XFER_MGR_WaitForRoutingInterfaceReady(&mtext->server_ip, timeout);


    if (FALSE == XFER_MGR_GetRemoteImageSWVersion(
                     mtext->server_type,
                     &(mtext->server_ip),
                     mtext->username,
                     mtext->password,
                     mtext->srcfile,
                     new_mainboard_version)
       )
    {
        printf("No new image detected\r\n");
        BUFFER_MGR_Free(mtext->buf);
        free(file_loc_p);

        XFER_MGR_SendAutoOpCodeUpgradeTrap(FALSE);
        return ;
    }

    XFER_MGR_Get_SWFile_Version(unit_id, old_mainboard_version, mtext->file_type);

    /* No need to upgrade
     */
    if (memcmp(old_mainboard_version, new_mainboard_version, sizeof(new_mainboard_version)) >= 0)
    {
        printf("No new image detected\r\n");
        BUFFER_MGR_Free(mtext->buf);
        free(file_loc_p);

        XFER_MGR_SendAutoOpCodeUpgradeTrap(FALSE);
        return;
    }

    printf("New image detected: current version %s; new version %s\r\n",
            old_mainboard_version, new_mainboard_version);

    mtext->auto_assign_dest_runtime_filename = TRUE;

    /* Set the new upgrade file as startup
     */
    mtext->is_next_boot = TRUE;

    /* Set callback function
     */
    mtext->cookie       = 0;
    mtext->callback     = XFER_MGR_UpgradeFileProgress_CallBack;

    printf("Image upgrade in progress\r\n");
    printf("Downloading new image\r\n");
    memset ( &info, 0, sizeof(Info_T) );
    XFER_MGR_CopyFile_Syn(mtext);

    free(file_loc_p);
    return ;
}/* End of XFER_MGR_UpgradeFile_Syn() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SendAutoOpCodeUpgradeTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send auto upgrade trap according to the upgrade result.
 * INPUT   : is_success -- Upgrade is success or not.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static void XFER_MGR_SendAutoOpCodeUpgradeTrap(BOOL_T is_success)
{
    TRAP_EVENT_TrapData_T trap_data;
    UI8_T cur_sw_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};

    memset(&trap_data, 0, sizeof(trap_data));

    trap_data.trap_type = TRAP_EVENT_XFER_AUTO_UPGRADE;
    trap_data.community_specified = FALSE;

    trap_data.u.auto_upgrade.instance_fileCopyFileType = 0;
    trap_data.u.auto_upgrade.fileCopyFileType = VAL_tftpFileType_opcode;

    if (TRUE == is_success)
    {
        XFER_MGR_Get_SWFile_Version(1, cur_sw_version, FS_TYPE_OPCODE_FILE_TYPE);

        trap_data.u.auto_upgrade.instance_autoUpgradeResult = 0;
        trap_data.u.auto_upgrade.autoUpgradeResult = VAL_trapAutoUpgradeResult_succeeded;
        trap_data.u.auto_upgrade.instance_autoUpgradeNewVer = 0;
        sprintf((char *)trap_data.u.auto_upgrade.autoUpgradeNewVer, "%s", cur_sw_version);
    }
    else
    {
        trap_data.u.auto_upgrade.instance_autoUpgradeResult = 0;
        trap_data.u.auto_upgrade.autoUpgradeResult = VAL_trapAutoUpgradeResult_failed;
    }

#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
    SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_UpgradeFileProgress_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: This function will be invoked by CLI provision completed
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static void XFER_MGR_UpgradeFileProgress_CallBack(void *cookie, UI32_T status)
{
    BOOL_T send_auto_upgrade_trap = FALSE;
    UI32_T reload_status = SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_RELOAD_STATUS;

    switch (status)
    {
        case XFER_MGR_FILE_COPY_SET_STARTUP_ERROR:
            printf("Set startup file error\r\n");
            break;

        case XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH:
             printf("Flash programming completed\r\n");
             break;

        case XFER_MGR_FILE_COPY_WRITE_FLASH_ERR:
             printf("Write to flash error\r\n");
             break;

        case XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING:
             printf("Flash programming started\r\n");
             break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT1:
            printf("Synchronizing to Unit 1\r\n");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT2:
            printf("Synchronizing to Unit 2\r\n");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT3:
            printf("Synchronizing to Unit 3\r\n");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT4:
            printf("Synchronizing to Unit 4\r\n");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT5:
            printf("Synchronizing to Unit 5\r\n");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT6:
            printf("Synchronizing to Unit 6\r\n");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT7:
            printf("Synchronizing to Unit 7\r\n");
            break;

        case XFER_MGR_FILE_COPY_PROGRESS_UNIT8:
            printf("Synchronizing to Unit 8\r\n");
            break;

        case XFER_MGR_FILE_COPY_SUCCESS:
             printf("Success\r\n");
            send_auto_upgrade_trap = TRUE;
            reload_status = XFER_OM_GetAutoOpCodeUpgradeReloadStatus();
            break;

        case XFER_MGR_FILE_COPY_ERROR:
            printf("Upgrade failed\r\n");
            send_auto_upgrade_trap = TRUE;
             break;

        case XFER_MGR_FILE_COPY_COMPLETED:
            printf("Upgrade completed\r\n");
            break;

        /* download success
         */
        case XFER_DNLD_TFTP_SUCCESS:
        case XFER_DNLD_TFTP_COMPLETED:
            break;

        /* download fail
         */
        default:
            printf("Downloading failed\r\n");
            break;
        }

    if (TRUE == send_auto_upgrade_trap)
    {
        XFER_MGR_SendAutoOpCodeUpgradeTrap(XFER_MGR_FILE_COPY_SUCCESS == status);
    }

    if (VAL_fileAutoUpgradeOpCodeReloadStatus_enabled == reload_status)
    {
        printf("The switch will now restart\r\n");

        /* Wait for send auto upgrade trap
         */
        if (TRUE == send_auto_upgrade_trap)
        {
            SYSFUN_Sleep(200);
        }

        STKCTRL_PMGR_ReloadSystem();

        while (1)
        {
            SYSFUN_Sleep(100);
        }
    }
}
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */

static void XFER_MGR_Print_BackdoorHelp(void)
{
    BACKDOOR_MGR_Printf("\n\t1 : Not Check Project ID");
    BACKDOOR_MGR_Printf("\n\t2 : Not Check Image Type (Runtime/Diag)");
    BACKDOOR_MGR_Printf("\n\t3 : Set TFTP Retry Times");
    BACKDOOR_MGR_Printf("\n\t4 : Turn on Debug flag (ftp debug = %lu)", (unsigned long)CMDFTP_GetDebugFlag());
    BACKDOOR_MGR_Printf("\n\t5 : Turn off Debug flag");
    BACKDOOR_MGR_Printf("\n\t6 : Show S/W Version");
    BACKDOOR_MGR_Printf("\n\t7 : Toggle ONIE installer debug message(%s)", (onie_installer_debug_flag==TRUE)?"On":"Off");
    BACKDOOR_MGR_Printf("\n\ta : Not Check Image Version");

    BACKDOOR_MGR_Printf("\n\tx : Exit TFTP Backdoor function");
    BACKDOOR_MGR_Printf("\n");
    return;
}

static void   XFER_MGR_BackdoorInfo_CallBack(void)
{
    UI8_T ch= 'x';
    BOOL_T  backdoor_continue ;
    UI32_T  retry_times=0;
    UI8_T   retry_times_buf[16];
    char    *terminal_p;
    UI32_T  unit_id, module_unit_id;

    L_THREADGRP_Handle_T    tg_handle;
    UI32_T                  backdoor_member_id;

    tg_handle = XFER_PROC_COMM_GetXFER_GROUPTGHandle();

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    BACKDOOR_MGR_Printf("\n TFTP Backdoor Selection");

    backdoor_continue = TRUE;

    while(backdoor_continue)
    {
        XFER_MGR_Print_BackdoorHelp();
        ch = BACKDOOR_MGR_GetChar();
        switch(ch)
        {
            case '1':
                BACKDOOR_MGR_Printf("\n Set not check Project ID ");
                XFER_MGR_CheckProjectId = FALSE ;
                break;

            case '2':
                BACKDOOR_MGR_Printf("\n Set not check Image Type ");
                XFER_MGR_CheckImageType = FALSE ;
                break;

            case '3':
                BACKDOOR_MGR_Printf ("\n");
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                XFER_MGR_GetTftpRetryTimes(&retry_times);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

                BACKDOOR_MGR_Printf("Input TFTP retry times[%lu]: ", (unsigned long)retry_times);
                BACKDOOR_MGR_RequestKeyIn((char *)retry_times_buf, 3);
                if(strcmp((char *)retry_times_buf, ""))
                {
                    retry_times = (int) strtoul( (char *)retry_times_buf, &terminal_p, 10);
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                    XFER_MGR_SetTftpRetryTimes(retry_times);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                }
                break;

            case '4':
                BACKDOOR_MGR_Printf("\nDebug flag is turned on\n");
                debug_flag = TRUE ;

                /* also set ftp debug flag
                 */
                CMDFTP_SetDebugFlag(CMDFTP_DBG_ERROR|CMDFTP_DBG_WARNING|CMDFTP_DBG_TRACE);
                break;

            case '5':
                BACKDOOR_MGR_Printf("\nDebug flag is turned off\n");
                debug_flag = FALSE ;

                /* also clear ftp debug flag
                 */
                CMDFTP_SetDebugFlag(0);
                break;

            case '6':
                for (unit_id = 1; unit_id <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id ++)
                {
                    UI8_T mainboard_ver_num[SYS_ADPT_FW_VER_STR_LEN +1] = {0};
                    UI8_T module_ver_num[SYS_ADPT_FW_VER_STR_LEN+1]    = {0};
                    if (TRUE == STKTPLG_POM_UnitExist(unit_id))
                    {
                        /* Get execution permission from the thread group handler if necessary
                         */
                        L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                        XFER_MGR_Get_SWFile_Version(unit_id, mainboard_ver_num, FS_TYPE_OPCODE_FILE_TYPE);
                        /* Release execution permission from the thread group handler if necessary
                         */
                        L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

                        if (TRUE == STKTPLG_POM_OptionModuleIsExist(unit_id, &module_unit_id))
                        {
                            /* Get execution permission from the thread group handler if necessary
                             */
                            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                            XFER_MGR_Get_SWFile_Version(module_unit_id, module_ver_num, FS_TYPE_OPCODE_FILE_TYPE);
                            /* Release execution permission from the thread group handler if necessary
                             */
                            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

                            BACKDOOR_MGR_Printf("\nUnit %ld: Main Board - %s, Module - %s\n", (long)unit_id, mainboard_ver_num, module_ver_num);
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("\nUnit %ld: Main Board - %s, No Module\n", (long)unit_id, mainboard_ver_num);
                        }
                    }
                }
                break;

            case '7':
                onie_installer_debug_flag=!onie_installer_debug_flag;
                break;
            case 'a':
                BACKDOOR_MGR_Printf("\n Set not check Image Version\n ");
                XFER_MGR_CheckImageVersion = FALSE ;
                break;

            case 'x':
            default:
                backdoor_continue = FALSE;

                L_THREADGRP_Leave(tg_handle, backdoor_member_id);
                return;
        }
    }

    return;
}/* End of XFER_MGR_BackDoorInfo() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_InitFileCopyMgtEntry
 *------------------------------------------------------------------------
 * FUNCTION: The function initialize file_copy_mgt_entry
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : If no copy operation has been performed, the status is XFER_MGR_UNKNOWN.
 *------------------------------------------------------------------------*/
static void XFER_MGR_InitFileCopyMgtEntry(void)
{
    memset (&file_copy_mgt_entry, 0, sizeof(XFER_MGR_FileCopyMgt_T));
    file_copy_mgt_entry.src_oper_type = VAL_fileCopySrcOperType_file;
    file_copy_mgt_entry.dest_oper_type = VAL_fileCopyDestOperType_file;
    file_copy_mgt_entry.file_type = FS_TYPE_OPCODE_FILE_TYPE;
    file_copy_mgt_entry.unit = 1;
    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_UNKNOWN;
    file_copy_mgt_entry.publickey = KEY_TYPE_RSA;

#if (SYS_CPNT_SFTP == TRUE)
    file_copy_mgt_entry.host_key_status = VAL_fileCopySftpHostkeyStatus_Deny;
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

    memset(&file_copy_mgt_entry.server_address, 0, sizeof(file_copy_mgt_entry.server_address));

    memset(file_copy_mgt_entry.username, 0, sizeof(file_copy_mgt_entry.username));
    memset(file_copy_mgt_entry.password, 0, sizeof(file_copy_mgt_entry.password));
}/* End of XFER_MGR_InitFileCopyMgtEntry() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFileFile
 *------------------------------------------------------------------------
 * FUNCTION: Copy from local file to local file
 * INPUT   : user_info_p    -- user information entry
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
 * NOTE    : If the type of file to copy is different from the source file's,
 *           will return FALSE.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyFileFile(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    UI32_T  unit_id;
    FS_File_Attr_T file_attr;
    L_INET_AddrIp_T ip_address;

    STKTPLG_POM_GetMyUnitID(&unit_id);
    memset(&file_attr, 0, sizeof(FS_File_Attr_T));
    memset(&ip_address, 0, sizeof(ip_address));

    if( (file_copy_mgt_entry.file_type < FS_FILE_TYPE_SUBFILE)
        || (file_copy_mgt_entry.file_type > FS_FILE_TYPE_TOTAL) )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILEFILE_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
        return FALSE;
    }

    if(file_copy_mgt_entry.file_type == FS_FILE_TYPE_CONFIG &&
       strcmp((char *)file_copy_mgt_entry.dest_file_name, SYS_DFLT_restartConfigFile) == 0)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILEFILE_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"filename");
        return FALSE;
    }

    file_attr.file_type_mask = FS_FILE_TYPE_MASK(file_copy_mgt_entry.file_type);
    strcpy((char *)file_attr.file_name, (char *)file_copy_mgt_entry.src_file_name);

    if(FS_RETURN_OK != FS_GetFileInfo(unit_id, &file_attr))
    {
        return FALSE;
    }

    if (!XFER_MGR_CopyFile(user_info_p,
                           file_copy_mgt_entry.publickey_username,
                           file_copy_mgt_entry.publickey,
                           &ip_address,
                           file_copy_mgt_entry.dest_file_name,
                           file_copy_mgt_entry.src_file_name,
                           file_copy_mgt_entry.file_type,
                           XFER_MGR_LOCAL_TO_LOCAL,
                           XFER_MGR_REMOTE_SERVER_NONE,
                           NULL, NULL,
                           cookie, ipc_message_q, callback))
    {
        return FALSE;
    }
    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_WriteFile_Syn
 *------------------------------------------------------------------------
 * FUNCTION: This function will save memory stream to local file
 * INPUT   : mtext - context of message of xfer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-----------------------------------------------------------------*/
static void XFER_MGR_WriteFile_Syn (Mtext_T *mtext)
{
    UI32_T ret;
    UI32_T unit_id;
    UI8_T  startup_file_name[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME + 1];
    char   original_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME + 1] = {0};

    memset(file_copy_mgt_entry.tftp_error_msg,0,sizeof(file_copy_mgt_entry.tftp_error_msg));

    STKTPLG_POM_GetMyUnitID(&unit_id);
    XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);

    if (mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE)
    {
        #if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
        image_header_t *imghdr;
        #endif
        UI8_T old_mainboard_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};
        UI8_T new_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};

        #if (SYS_CPNT_ONIE_SUPPORT==TRUE)
        if (XFER_MGR_GetSWVersionFromONIEInstallerStream(mtext->buf, new_version)==FALSE)
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            mtext->status = file_copy_mgt_entry.status;
            XFER_MGR_SendFileCopyTrap(mtext);
            is_file_copy_mgt_used = FALSE;
            return;
        }
        #else
        imghdr = (image_header_t *)mtext->buf;
        memcpy(new_version, imghdr->ih_ver, SYS_ADPT_FW_VER_STR_LEN+1);
        #endif

        if( (XFER_MGR_IsCheckImageVersion() == TRUE)
            && XFER_MGR_Get_SWFile_Version(unit_id, old_mainboard_version, FS_TYPE_OPCODE_FILE_TYPE)
            && (0 == strcmp((char *)old_mainboard_version, (char *)new_version)) )
        {
            if ((FS_RETURN_OK == FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
                && (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile))
               )
            {
                /* save original source filename cause it will be change when sync to slave
                 */
                strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                original_filename[sizeof(original_filename)-1] = '\0';

                /* sync to slaves
                 */
                mtext->unit_id = unit_id;
                strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                XFER_MGR_AutoDownLoad_Syn(mtext);

                /* revert to original source filename
                 */
                strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
            }

            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SAME_VERSION);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SAME_VERSION;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            mtext->status = file_copy_mgt_entry.status;
            XFER_MGR_SendFileCopyTrap(mtext);
            is_file_copy_mgt_used = FALSE;
            return;
        }
    }

#if (SYS_CPNT_SINGLE_RUNTIME_IMAGE == TRUE)
    /* For projects that have different max number of runtime image on different
     * board ids(For example, ES3510MA-FLF-38), need to check the real runtime
     * partition number to determine
     * whether the code snippet shown below should be run
     */
    if(FS_GetNumOfPartitionByTypeFromPartitionTable(TYPE_RUNTIME)==1)
    {
        /* For single image case: Use new opcode name as startup opcode name.
         * Before writing new opcode, to get startup opcode and then delete it.
         */
        if (FS_TYPE_OPCODE_FILE_TYPE == mtext->file_type)
        {
            mtext->is_next_boot = TRUE;

            if(FS_RETURN_OK != FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
                mtext->status = file_copy_mgt_entry.status;
                XFER_MGR_SendFileCopyTrap(mtext);
                is_file_copy_mgt_used = FALSE;
                return;
            }

            if(0 != strcmp((char *)startup_file_name, (char *)mtext->destfile) )
            {
                /* 1. Check available free space.
                 * 2. Reset fs startup table.
                 * 3. Delete current startup opcode.
                 */
                if (   (FALSE == XFER_MGR_CheckFreeSizeByFileType(FS_TYPE_OPCODE_FILE_TYPE, unit_id, startup_file_name, mtext->length))
                    || (FS_RETURN_OK != FS_DeleteFile(unit_id, startup_file_name))
                    )
                {
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
                    mtext->status = file_copy_mgt_entry.status;
                    XFER_MGR_SendFileCopyTrap(mtext);
                    is_file_copy_mgt_used = FALSE;
                    return;
                }
            }
        }
    }
#endif  /*#if (SYS_CPNT_SINGLE_RUNTIME_IMAGE == TRUE)*/

#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
    if (FS_FILE_TYPE_TOTAL == mtext->file_type)
    {
        ret = FS_WriteLoader(unit_id, mtext->buf+sizeof(LDR_INFO_BLOCK_T), mtext->length-sizeof(LDR_INFO_BLOCK_T));

        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);

        if(FS_RETURN_OK == ret)
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
        }
        else
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;

        } /* End of if (FS_WriteLoader(...)) */
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
     }
    else
#endif /* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */
    {
        ret = FS_WriteFile(unit_id,mtext->destfile, (UI8_T *)"Xfer", mtext->file_type, mtext->buf, mtext->length,0);

        if (ret == FS_RETURN_OK)
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
            if(TRUE == mtext->is_next_boot &&
               FS_RETURN_OK != FS_SetStartupFilename(unit_id, mtext->file_type, mtext->destfile))
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SET_STARTUP_ERROR);

                file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SET_STARTUP_ERROR;
                file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            }
            else
            {
                /* if the destination file is current startup file then sync to slave
                 */
                if (    (FS_RETURN_OK == FS_GetStartupFilename(unit_id, mtext->file_type, startup_file_name))
                    &&  (0 == strcmp((char *)startup_file_name, (char *)mtext->destfile))
                   )
                {
                    /* save original source filename cause it will be change when sync to slave
                     */
                    strncpy(original_filename, (char *)mtext->srcfile, sizeof(original_filename)-1);
                    original_filename[sizeof(original_filename)-1] = '\0';

                    /* sync to slaves
                     */
                    mtext->unit_id = unit_id;
                    strncpy((char *)mtext->srcfile, (char *)mtext->destfile, sizeof(mtext->srcfile)-1);
                    mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                    XFER_MGR_AutoDownLoad_Syn(mtext);

                    /* revert to original source filename
                     */
                    strncpy((char *)mtext->srcfile, original_filename, sizeof(mtext->srcfile)-1);
                    mtext->srcfile[sizeof(mtext->srcfile)-1] = '\0';
                }

                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);

                    file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_SUCCESS;
                    file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            }
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        }
        else
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);

            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        }
    }

    mtext->status = file_copy_mgt_entry.status;
    XFER_MGR_SendFileCopyTrap(mtext);
    is_file_copy_mgt_used = FALSE;

    return ;
}/* End of XFER_MGR_StreamToLocal_Syn() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFileRunning
 *------------------------------------------------------------------------
 * FUNCTION: Copy from local file to running conifg
 * INPUT   : user_info_p    -- user information entry
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
 * NOTE    : After completed copy operation, will provision.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyFileRunning(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    UI8_T  *xfer_buf;
    UI32_T xbuf_length;

    if((xfer_buf = (UI8_T *)BUFFER_MGR_Allocate()) == NULL)
	{
	    return FALSE;
	}

    memset(xfer_buf, 0, SYS_ADPT_MAX_FILE_SIZE);
    strcpy((char *)xfer_buf, CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD);
    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
    if (XFER_MGR_LocalToStream(user_info_p,
                               file_copy_mgt_entry.src_file_name,
                               xfer_buf + strlen(CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD),
                               &xbuf_length,
                               SYS_ADPT_MAX_FILE_SIZE)!=FS_RETURN_OK )
    {
        return FALSE;
    }
    else
    {
        CLI_PMGR_Notify_EnterTransitionMode(xfer_buf); /* notify to enter transition mode */
    }
    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFileRemote
 *------------------------------------------------------------------------
 * FUNCTION: Copy from local file to remote
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
 * NOTE    : If the type of file to copy is different from the source file's,
 *           will return FALSE.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyFileRemote(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    UI32_T  unit_id;
    FS_File_Attr_T file_attr;

    STKTPLG_POM_GetMyUnitID(&unit_id);
    memset(&file_attr, 0, sizeof(FS_File_Attr_T));

    if( (file_copy_mgt_entry.file_type < FS_FILE_TYPE_SUBFILE)
        || (file_copy_mgt_entry.file_type > FS_FILE_TYPE_TOTAL) )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILETFTP_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
        return FALSE;
    }

    file_attr.file_type_mask = FS_FILE_TYPE_MASK(file_copy_mgt_entry.file_type);
    strcpy((char *)file_attr.file_name, (char *)file_copy_mgt_entry.src_file_name);

    if(FS_RETURN_OK != FS_GetFileInfo(unit_id, &file_attr))
    {
        return FALSE;
    }

    if (!XFER_MGR_CopyFile(user_info_p,
                           file_copy_mgt_entry.publickey_username,
                           file_copy_mgt_entry.publickey,
                           &file_copy_mgt_entry.server_address,
                           file_copy_mgt_entry.dest_file_name,
                           file_copy_mgt_entry.src_file_name,
                           file_copy_mgt_entry.file_type,
                           XFER_MGR_LOCAL_TO_REMOTE,
                           server_type,
                           file_copy_mgt_entry.username,
                           file_copy_mgt_entry.password,
                           cookie, ipc_message_q, callback))
    {
        return FALSE;
    }
    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRunningFile
 *------------------------------------------------------------------------
 * FUNCTION: Copy from running config to local file
 * INPUT   : user_info_p    -- user information entry
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
static BOOL_T
XFER_MGR_CopyRunningFile(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    if(strcmp((char *)file_copy_mgt_entry.dest_file_name, SYS_DFLT_restartConfigFile) == 0)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYRUNNINGFILE_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"filename");
        return FALSE;
    }

    if (!XFER_MGR_StreamToLocal(user_info_p,
                                file_copy_mgt_entry.dest_file_name,
                                file_copy_mgt_entry.file_type,
                                cookie, ipc_message_q, callback))
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRunningRemote
 *------------------------------------------------------------------------
 * FUNCTION: Copy from running config to tftp
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
static BOOL_T
XFER_MGR_CopyRunningRemote(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    if (!XFER_MGR_StreamToRemote(user_info_p,
                                 server_type,
                                 &file_copy_mgt_entry.server_address,
                                 file_copy_mgt_entry.username,
                                 file_copy_mgt_entry.password,
                                 file_copy_mgt_entry.dest_file_name,
                                 file_copy_mgt_entry.file_type,
                                 cookie, ipc_message_q, callback))
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRemoteFile
 *------------------------------------------------------------------------
 * FUNCTION: Copy from remote to local file
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
static BOOL_T
XFER_MGR_CopyRemoteFile(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    if(file_copy_mgt_entry.file_type == FS_FILE_TYPE_CONFIG &&
       strcmp((char *)file_copy_mgt_entry.dest_file_name, SYS_DFLT_restartConfigFile) == 0)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYTFTPFILE_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"filename");
        return FALSE;
    }

#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
    if(file_copy_mgt_entry.file_type == FS_FILE_TYPE_TOTAL)
    {
        file_copy_mgt_entry.dest_file_name[0] = 'L';
        file_copy_mgt_entry.dest_file_name[1] = '\0';
    }
#endif  /* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */

    if (!XFER_MGR_CopyFile(user_info_p,
                           file_copy_mgt_entry.publickey_username,
                           file_copy_mgt_entry.publickey,
                           &file_copy_mgt_entry.server_address,
                           file_copy_mgt_entry.dest_file_name,
                           file_copy_mgt_entry.src_file_name,
                           file_copy_mgt_entry.file_type,
                           XFER_MGR_REMOTE_TO_LOCAL,
                           server_type,
                           file_copy_mgt_entry.username,
                           file_copy_mgt_entry.password,
                           cookie, ipc_message_q, callback))
    {
        return FALSE;
    }
    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyRemoteRunning
 *------------------------------------------------------------------------
 * FUNCTION: Copy from remote to running
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
 * NOTE    : After completed copy operation, will provision.
 *------------------------------------------------------------------------
 */
static
BOOL_T XFER_MGR_CopyRemoteRunning(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    UI8_T  *xfer_buf;


	if((xfer_buf = (UI8_T *)BUFFER_MGR_Allocate()) == NULL)
	{
	    return FALSE;
	}

    memset(xfer_buf, 0, SYS_ADPT_MAX_FILE_SIZE);

    strcpy((char *)xfer_buf, CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD);
    info.is_end_with_provision = TRUE;
    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;
    if (!XFER_MGR_RemoteToStream(user_info_p,
                                 server_type,
                                 &file_copy_mgt_entry.server_address,
                                 file_copy_mgt_entry.username,
                                 file_copy_mgt_entry.password,
                                 file_copy_mgt_entry.src_file_name,
                                 file_copy_mgt_entry.file_type,
                                 xfer_buf + strlen(CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD),
                                 cookie, ipc_message_q, callback,
                                 SYS_ADPT_MAX_FILE_SIZE))
    {
        return FALSE;
    }
    return TRUE;
}

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_AddRemoteRunning
 *------------------------------------------------------------------------
 * FUNCTION: Add remote config to running
 * INPUT   : user_info_p    -- user information entry
 *           server_type    -- remote server type
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
 * NOTE    :
 *------------------------------------------------------------------------*/
static BOOL_T
XFER_MGR_AddRemoteRunning(
    XFER_MGR_UserInfo_T *user_info_p,
    XFER_MGR_RemoteServer_T server_type,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    UI8_T  *xfer_buf;

    if (server_type == XFER_MGR_REMOTE_SERVER_NONE)
    {
        return FALSE;
    }

    if ((xfer_buf = (UI8_T *)BUFFER_MGR_Allocate()) == NULL)
    {
        return FALSE;
    }

    memset(xfer_buf, 0, SYS_ADPT_MAX_FILE_SIZE);
    strcpy((char *)xfer_buf, CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD);
    info.is_partial_provision = TRUE;
    file_copy_mgt_entry.file_type = FS_FILE_TYPE_CONFIG;

    if (TRUE != XFER_MGR_RemoteToStream(user_info_p,
                                        server_type,
                                        &file_copy_mgt_entry.server_address,
                                        file_copy_mgt_entry.username,
                                        file_copy_mgt_entry.password,
                                        file_copy_mgt_entry.src_file_name,
                                        file_copy_mgt_entry.file_type,
                                        xfer_buf + strlen(CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD),
                                        cookie,
                                        ipc_message_q,
                                        callback,
                                        SYS_ADPT_MAX_FILE_SIZE))
    {
        return FALSE;
    }

    return TRUE;
}
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyUnitFile
 *------------------------------------------------------------------------
 * FUNCTION: Copy from other unit file to local file
 * INPUT   : user_info_p    -- user information entry
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
 * NOTE    : The type of file to copy must be FS_FILE_TYPE_CONFIG or FS_TYPE_OPCODE_FILE_TYPE.
 *           The destination file couldn't be factory default config.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyUnitFile(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    if(file_copy_mgt_entry.file_type != FS_FILE_TYPE_CONFIG &&
       file_copy_mgt_entry.file_type != FS_TYPE_OPCODE_FILE_TYPE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYUNITFILE_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
        return FALSE;
    }

    if(file_copy_mgt_entry.file_type == FS_FILE_TYPE_CONFIG &&
       strcmp((char *)file_copy_mgt_entry.dest_file_name, SYS_DFLT_restartConfigFile) == 0)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYUNITFILE_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"filename");
        return FALSE;
    }

    if (XFER_MGR_UnitToLocal(user_info_p,
                             file_copy_mgt_entry.unit,
                             file_copy_mgt_entry.dest_file_name,
                             file_copy_mgt_entry.src_file_name,
                             file_copy_mgt_entry.file_type,
                             cookie, ipc_message_q, callback) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyFileUnit
 *------------------------------------------------------------------------
 * FUNCTION: Copy from local file to other unit
 * INPUT   : user_info_p    -- user information entry
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
 * NOTE    : The type of file to copy must be FS_FILE_TYPE_CONFIG or FS_TYPE_OPCODE_FILE_TYPE.
 *           The destination file couldn't be factory default config.
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_CopyFileUnit(
    XFER_MGR_UserInfo_T *user_info_p,
    void *cookie,
    UI32_T ipc_message_q,
    void (*callback) (void *cookie, UI32_T status))
{
    if(file_copy_mgt_entry.file_type != FS_FILE_TYPE_CONFIG &&
       file_copy_mgt_entry.file_type != FS_TYPE_OPCODE_FILE_TYPE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILEUNIT_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"type");
        return FALSE;
    }

    if(file_copy_mgt_entry.file_type == FS_FILE_TYPE_CONFIG &&
       strcmp((char *)file_copy_mgt_entry.dest_file_name, SYS_DFLT_restartConfigFile) == 0)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILEUNIT_FUNC_NO,
        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,"filename");
        return FALSE;
    }

    if (XFER_MGR_LocalToUnit(user_info_p,
                             file_copy_mgt_entry.unit,
                             file_copy_mgt_entry.dest_file_name,
                             file_copy_mgt_entry.src_file_name,
                             file_copy_mgt_entry.file_type,
                             cookie, ipc_message_q, callback) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_AutoConfigToUnit_Syn
 *------------------------------------------------------------------------
 * FUNCTION: The function copy file from local to unit
 * INPUT   : XFER_MGR_Msg_T *mtext
 * OUTPUT  :
 * RETURN  : none
 * NOTE    : Must free mtext
 *------------------------------------------------------------------------*/
static void     XFER_MGR_AutoConfigToUnit_Syn (Mtext_T *mtext)
{
    UI32_T              read_count;
    FS_File_Attr_T      file_attr;
    UI32_T              i = 0;
    UI32_T              unit_id = 0;
    UI8_T               copy_unit_list[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK] = {0};/*need copy unit(exclude master)*/
    UI32_T              callback_status = XFER_MGR_FILE_COPY_UNKNOWN;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /*add callback status ???*/
        BUFFER_MGR_Free(mtext->buf);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);


        SYSFUN_Debug_Printf("XFER: End config sync, but operation mode is wrong.\r\n");
        return;/*if topology change must stop action until flash writing finish*/
    }

    file_attr.file_type_mask = FS_FILE_TYPE_MASK_ALL;
    strcpy ( (char *)file_attr.file_name, (char *)mtext->srcfile);
    STKTPLG_POM_GetMyUnitID(&unit_id);
    memset(copy_unit_list, 0, sizeof(copy_unit_list));
    XFER_MGR_InitAutoDownloadStatus();
    if( FS_GetFileInfo(mtext->unit_id, &file_attr)!= FS_RETURN_OK ||
        FS_ReadFile( mtext->unit_id, file_attr.file_name,  mtext->buf, SYS_ADPT_MAX_FILE_SIZE, &read_count) != FS_RETURN_OK)
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_READ_FILE_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        auto_download_unit[(mtext->unit_id)-1].auto_download_status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
    }
    else
    {
        mtext->length=read_count;

        if (mtext->file_type != file_attr.file_type)
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            auto_download_unit[(mtext->unit_id)-1].auto_download_status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
        }
        else
        {
            /*force unit one(master) as success*/
            auto_download_unit[unit_id-1].auto_download_status = XFER_MGR_FILE_COPY_SUCCESS;
            if (FALSE == XFER_MGR_Check_File_Version(copy_unit_list, mtext->file_type))
            {
                for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                {
                    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
                    {
                        /*add callback status ???*/
                        break;/*if topology change must stop action until flash writing finish*/
                    }
                    if (1 == copy_unit_list[i - 1])
                    {
                        SYSFUN_Debug_Printf("XFER: unit: %lu, begin\r\n", (unsigned long)i);

                        /*if have 2 opcode delete non startup file first
                          This is becuase file name may not the same as
                          download file, and we only have 2(now) opcode
                          if only check size and write flash directly may
                          have error*/
                        XFER_MGR_Delete_NonStartOPcode(i, mtext->destfile, mtext->file_type);
                        if (FALSE == XFER_MGR_CheckFreeSizeByFileType(FS_FILE_TYPE_CONFIG, i, mtext->destfile,read_count))
                        {
                            if (TRUE != XFER_MGR_Delete_FileForCopy(i, read_count))
                            {
                                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
                                auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
                                continue;
                            }
                        }

                        switch(i)
                        {
                        case 1:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT1;
                            break;

                        case 2:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT2;
                            break;

                        case 3:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT3;
                            break;

                        case 4:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT4;
                            break;

                        case 5:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT5;
                            break;

                        case 6:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT6;
                            break;

                        case 7:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT7;
                            break;

                        case 8:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT8;
                            break;

                        default:
                            callback_status = XFER_MGR_FILE_COPY_ERROR;
                            break;
                        }
                        XFER_MGR_CallBack (mtext, callback_status);
                        XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);
                        callback_status = XFER_MGR_FILE_COPY_SUCCESS;
                        auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING;
                        auto_download_unit[i-1].copy_action = VAL_fileCopyAction_copy;

                        if (strcmp((char *)mtext->destfile, SYS_DFLT_restartConfigFile) == 0 && mtext->file_type == FS_FILE_TYPE_CONFIG)
                        {
                            if (TRUE == mtext->is_next_boot)
                            {
                                FS_SetStartupFilename(i, mtext->file_type, mtext->destfile);
                            }
                            auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_SUCCESS;
                            auto_download_unit[i-1].copy_action = VAL_fileCopyAction_notCopying;
                        }
                        else
                        {
                            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);

                            if(FS_WriteFile (i, mtext->destfile, (UI8_T *)"Xfer", file_attr.file_type, mtext->buf, read_count,0) == FS_RETURN_OK )
                            {
                                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
                                if (TRUE == mtext->is_next_boot)
                                {
                                    FS_SetStartupFilename(i, mtext->file_type, mtext->destfile);
                                }
                                auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_SUCCESS;
                                auto_download_unit[i-1].copy_action = VAL_fileCopyAction_notCopying;
                            }
                            else
                            {
                                auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                                auto_download_unit[i-1].copy_action = VAL_fileCopyAction_notCopying;
                                callback_status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                                printf("\r\nError: Unit %lu config sync failed.\r\n", (unsigned long)i);
                            }/*end of finish writing*/
                        }
                        SYSFUN_Debug_Printf("XFER: unit: %lu, end\r\n", (unsigned long)i);
                    }/*end of if copy unit*/
                }/*end of unit loop*/
            }/*end of check version*/
        }/*end of if file type not the same*/

/* sync $stacking_DB at system startup time is not necessary, since unit table has been
 * moved to CLI config file and STKTPLG will no longer create this file.
 */
#if 0
        /*temp solution for sync unit id, always sync unit id after sync config file*/
        memset(mtext->buf, 0, sizeof(UI8_T)*SYS_ADPT_MAX_FILE_SIZE);
        if (FALSE == XFER_MGR_Check_File_Version(copy_unit_list, FS_FILE_TYPE_PRIVATE))
        {
            if (FS_ReadFile( mtext->unit_id, "$stacking_DB",  mtext->buf, SYS_ADPT_MAX_FILE_SIZE, &read_count) == FS_RETURN_OK)
            {
                mtext->length=read_count;
                for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                {
                    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
                    {
                        /*add callback status ???*/
                        break;/*if topology change must stop action until flash writing finish*/
                    }
                    if (1 == copy_unit_list[i - 1])
                    {
                            switch(i)
                            {
                            case 1:
                                callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT1;
                                break;

                            case 2:
                                callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT2;
                                break;

                            case 3:
                                callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT3;
                                break;

                            case 4:
                                callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT4;
                                break;

                            case 5:
                                callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT5;
                                break;

                            case 6:
                                callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT6;
                                break;

                            case 7:
                                callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT7;
                                break;

                            case 8:
                                callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT8;
                                break;

                            default:
                                callback_status = XFER_MGR_FILE_COPY_ERROR;
                                break;
                            }
                            XFER_MGR_CallBack (mtext, callback_status);
                            XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);
                        if(FS_WriteFile (i, (UI8_T *)"$stacking_DB", (UI8_T *)"Xfer", FS_FILE_TYPE_PRIVATE, mtext->buf, read_count,0) == FS_RETURN_OK )
                        {
                            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
                        }
                    }/*end of if copy unit*/
                }/*end of unit loop*/
            }
        }/*end if check version*/
#endif /* end of #if 0*/

        XFER_MGR_CallBack(mtext, callback_status);
        file_copy_mgt_entry.status = callback_status;
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
    }
    BUFFER_MGR_Free(mtext->buf);


    SYSFUN_Debug_Printf("XFER: End of config sync.\r\n");
    return ;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_AutoDownLoad_Syn
 *------------------------------------------------------------------------
 * FUNCTION: The function copy file from local to unit
 * INPUT   : XFER_MGR_Msg_T *mtext
 * OUTPUT  :
 * RETURN  : none
 * NOTE    : Must free mtext
 *------------------------------------------------------------------------*/
static void XFER_MGR_AutoDownLoad_Syn (Mtext_T *mtext)
{
    UI32_T                    read_count;
    FS_File_Attr_T            file_attr;
    UI32_T                    i = 0;
    UI8_T                     copy_unit_list[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK] = {0}; /* need copy unit(exclude master) */
    UI32_T                    callback_status = 0;
    UI32_T                    my_unit = 0;
    UI32_T                    fail_unit_count = 0;
    UI32_T                    retry_time = 0;
    BOOL_T                    is_retry = FALSE;
    UI32_T                    result;
    UI16_T                    copy_unit_bmp = 0;
    UI32_T                    test_unit_id = 0;
    BOOL_T                    ret = FS_RETURN_ERROR;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return; /* if topology change must stop action until flash writing finish */
    }
    memset(&file_attr, 0, sizeof(file_attr));

    memset(copy_unit_list, 0, sizeof(copy_unit_list));

    STKTPLG_POM_GetMyUnitID(&my_unit);

    if (auto_download_sync_to_master == TRUE)
    {
        if (mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE)
        {
            /* There is no way to know the length of the runtime image header on
             * ONIE environment. For better support between ONIE and non-ONIE
             * enviroment, the caller shall always set mtext->length for the
             * length of the runtime image.
             */
            #if 1
            if (mtext->length==0)
            {
                BACKDOOR_MGR_Printf("%s(%d)mtext->length should not be 0!\r\n", __FUNCTION__, __LINE__);
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_UNKNOWN);
                auto_download_unit[(mtext->unit_id)-1].auto_download_status = XFER_MGR_FILE_COPY_UNKNOWN;
                return;
            }
            else
            {
                read_count = mtext->length;
                copy_unit_list[my_unit-1] = 1;
            }
            #else /* obsoleted code for reference only */
            /* read image header and image length from mtext file buffer
             */
            image_header_t   *imghdr;
            imghdr = (image_header_t *)mtext->buf;
            if(imghdr->ih_magic != IH_MAGIC)
            {
                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_MAGIC_WORD_ERROR);
                auto_download_unit[(mtext->unit_id)-1].auto_download_status = XFER_MGR_FILE_COPY_MAGIC_WORD_ERROR;
                return;

            }
            else
            {
                read_count = imghdr->ih_size + sizeof(image_header_t);
                copy_unit_list[my_unit-1] = 1;
            }
            #endif /* end of #if 1 */
        }
        else if (mtext->file_type == FS_FILE_TYPE_CONFIG)
        {
            read_count = mtext->length;
            copy_unit_list[my_unit-1] = 1;
        }
    }
    else
    {
        copy_unit_list[my_unit-1] = 0;
        /*when master not need syn, file must be in master dut, metxt->srcfile is the file need syn to slave.*/
        if (FS_ReadFile(mtext->unit_id, mtext->srcfile,  mtext->buf, SYS_ADPT_MAX_FILE_SIZE,
                          &read_count) != FS_RETURN_OK)
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            auto_download_unit[(mtext->unit_id)-1].auto_download_status = XFER_MGR_FILE_COPY_READ_FILE_ERROR;
            return;
        }
        mtext->length=read_count;
    }
#if 0
     /*if dst file is default config file ....*/
     if (strcmp((char *)mtext->destfile, SYS_DFLT_restartConfigFile) == 0 && mtext->file_type == FS_FILE_TYPE_CONFIG)
    {
        XFER_MGR_Check_File_Version_By_MText(copy_unit_list, mtext);
        for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
        {
            if (1 == copy_unit_list[i - 1])
            {
                if (TRUE == mtext->is_next_boot)
                {
                    FS_SetStartupFilename(i, mtext->file_type, mtext->destfile);
                }
                auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_SUCCESS;
                auto_download_unit[i-1].copy_action = VAL_fileCopyAction_notCopying;
            }
        }
    }
#endif
    if(mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE || mtext->file_type == FS_FILE_TYPE_CONFIG)
    {
        if(XFER_MGR_Check_File_Version_By_MText(copy_unit_list, mtext) != TRUE
            || auto_download_sync_to_master)
        {


            while ((0 == retry_time)
                   || (retry_time <= XFER_MGR_AUTO_SYNC_NUMBER_OF_RETRIES && is_retry == TRUE)
                  )
            {
                if (is_retry)
                {
                    if (XFER_MGR_IsDebugFlagOn())
                    {
                        BACKDOOR_MGR_Printf("\r\nStarting to retry, there are still %lu unit need upgrade.\r\n", (unsigned long)fail_unit_count);
                        for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                        {
                            if (1 == copy_unit_list[i - 1])
                            {
                                BACKDOOR_MGR_Printf("Unit %lu\r\n", (unsigned long)i);
                            }
                        }
                    }
                }

                is_retry = FALSE;
                retry_time ++;
                fail_unit_count = 0;

                for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i ++)
                {
                    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
                    {
                        /* add callback status ??? */
                        break; /* if topology change must stop action until flash writing finish */
                    }
                    switch(i)
                    {
                        case 1:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT1;
                            break;

                        case 2:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT2;
                            break;

                        case 3:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT3;
                            break;

                        case 4:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT4;
                            break;

                        case 5:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT5;
                            break;

                        case 6:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT6;
                            break;

                        case 7:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT7;
                            break;

                        case 8:
                            callback_status = XFER_MGR_FILE_COPY_PROGRESS_UNIT8;
                            break;

                        default:
                            callback_status = XFER_MGR_FILE_COPY_ERROR;
                            break;
                    }

                    if (1 == copy_unit_list[i - 1])
                    {
                        XFER_MGR_CallBack (mtext, callback_status);
                        XFER_MGR_CallBack (mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING);


                        /* a.check runtime/config file num, ....*/
                        if(FS_GetNumOfFileByUnitID(i, mtext->file_type) >=( mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE
							                        ? FS_TYPE_MAX_NUM_OF_FILE_OP_CODE : SYS_ADPT_MAX_NUM_OF_FILE_CONFIG)
                            && i != my_unit /*if master file num exceed, we can not auto delete*/)
                        {
						    UI32_T unit_id = i;
                            memset(&file_attr, 0, sizeof(file_attr));

							file_attr.file_type_mask = FS_FILE_TYPE_MASK(mtext->file_type);

                            while(FS_GetNextFileInfo(&unit_id ,&file_attr) == FS_RETURN_OK && unit_id == i)
                            {
                                if((file_attr.startup_file != TRUE )
                                && (strcmp((char*)file_attr.file_name, SYS_DFLT_restartConfigFile) != 0))
                                {
                                    ret = FS_DeleteFile(i, file_attr.file_name);
                                    break;
                                }

                            }
                            if(ret != FS_RETURN_OK)
                            {
                                fail_unit_count++;
                                is_retry = TRUE;
                                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
                                auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                                if (XFER_MGR_AUTO_SYNC_NUMBER_OF_RETRIES == (retry_time - 1))
                                {
                                    printf("\r\nError: Unit %lu failed to delete file for copy.\r\n", (unsigned long)i);
                                }
                                continue;
                            }


                        }



                        auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING;
                        auto_download_unit[i-1].copy_action = VAL_fileCopyAction_copy;
                    }
                }
                /* start to write(sync) file to main board exclude factory default config file,
                                 * factory default config file is only need to set it as startup.
                                */
			    if (strcmp((char *)mtext->destfile, SYS_DFLT_restartConfigFile) == 0 && mtext->file_type == FS_FILE_TYPE_CONFIG)
			    {
			        //XFER_MGR_Check_File_Version_By_MText(copy_unit_list, mtext);
			        for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
			        {
			            if (1 == copy_unit_list[i - 1])
			            {
			       			FS_SetStartupFilename(i, mtext->file_type, mtext->destfile);
			   			}
			        }
					if (fail_unit_count > 0)
				    {
				        callback_status = XFER_MGR_FILE_COPY_ERROR;
				    }
				    else
				    {
				        callback_status = XFER_MGR_FILE_COPY_SUCCESS;
				    }
			        XFER_MGR_CallBack(mtext, callback_status);
			        file_copy_mgt_entry.status = callback_status;
					return;
			    }

                  /*1.  translate copy_unit_list to copy_unit_bmp */
                for (test_unit_id = 1; test_unit_id <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; test_unit_id ++)
                {
                    if (1 == copy_unit_list[test_unit_id - 1])
                    {
                        copy_unit_bmp |= ((0x01) << (test_unit_id - 1));
                    }
                }
                /*2. write to multiple unit according unit bitmap*/
#if (SYS_CPNT_STACKING == TRUE)
                if (TRUE != FS_WriteFileToMultipleUnit(&copy_unit_bmp,  mtext->destfile,
                                                                   (UI8_T *)"Xfer", mtext->file_type,
                                                                   mtext->buf,      read_count, 0))
                {
                    printf("%s, FS_WriteFileToMultipleUnit return FALSE, \r\n", __FUNCTION__);
                    return;
                }
#endif
                /* 3 .after write file to main board completely, set new file as startup */
                for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i ++)
                {
                    if (1 == copy_unit_list[i - 1])
                    {
                        if (!(copy_unit_bmp & ((0x01) << (i - 1))))
                        {
                            copy_unit_list[i - 1] = 0;
                            if (TRUE == mtext->is_next_boot)
                            {
                                result = FS_SetStartupFilename(i, mtext->file_type, mtext->destfile);
                                if(FS_RETURN_OK != result)
                                {
                                    auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_SET_STARTUP_ERROR;
                                    XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SET_STARTUP_ERROR);
                                }
                            }
                        }
                        else
                        {
                            auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_WRITE_FLASH_ERR;
                            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_ERR);
							fail_unit_count++;
                        }
                    }
                }

                XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH);
            }

            if (fail_unit_count > 0)
            {
                callback_status = XFER_MGR_FILE_COPY_ERROR;
            }
            else
            {
                callback_status = XFER_MGR_FILE_COPY_SUCCESS;
            }
            XFER_MGR_CallBack(mtext, callback_status);
            file_copy_mgt_entry.status = callback_status;
        }
        else
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_SUCCESS);
            auto_download_unit[(mtext->unit_id)-1].auto_download_status = XFER_MGR_FILE_COPY_SUCCESS;
        }
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CheckFreeSizeByFileType
 *------------------------------------------------------------------------
 * FUNCTION: The function is check flash have enough space to write
 * INPUT   : unit_id -> unid_id, file_name -> download filename, file_size -> download file length
 * OUTPUT  :
 * RETURN  : TRUE (enough) / FALSE (no enough)
 * NOTE    : none
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_CheckFreeSizeByFileType(FS_File_Type_T file_type,
    UI32_T unit_id, UI8_T *file_name, UI32_T file_size)
{
    UI32_T  total_size_of_free_space = 0;
    UI32_T  exist_size = 0;
    FS_File_Attr_T file_attr;

    memset(&file_attr, 0, sizeof(FS_File_Attr_T));
    strcpy ( (char *)file_attr.file_name, (char *)file_name);

    if (file_type == FS_TYPE_OPCODE_FILE_TYPE)
    {
        if (FS_GetStorageFreeSpaceForUpdateRuntime(unit_id,
            &total_size_of_free_space) != FS_RETURN_OK)
        {
            return FALSE;
        }

        file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_TYPE_OPCODE_FILE_TYPE);
        if (FS_GetFileInfo(unit_id, &file_attr) == FS_RETURN_OK)
        {
            exist_size = file_attr.storage_size;
        }
    }
    else
    {
        if (FS_GetStorageFreeSpace(unit_id, &total_size_of_free_space) !=
            FS_RETURN_OK)
        {
            return FALSE;
        }

        file_attr.file_type_mask = FS_FILE_TYPE_MASK_ALL;
        if (FS_GetFileInfo(unit_id,& file_attr) == FS_RETURN_OK)
        {
            exist_size = file_attr.file_size;
        }
    }

    if ((total_size_of_free_space + exist_size) < file_size)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Delete_FileForCopy
 *------------------------------------------------------------------------
 * FUNCTION: The function is to delete file let auto copy can done well
 * INPUT   : unit_id -> unid_id, file_size -> download file length
 * OUTPUT  :
 * RETURN  : TRUE (size enough / delete ok) / FALSE (delete fail)
 * NOTE    : 1.Delete non startup runtime code at first.
 *           2.Delete non startup config file at second.
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_Delete_FileForCopy(UI32_T unit_id, UI32_T file_size)
{
    UI32_T  total_size_of_free_space = 0;
    UI32_T  now_unit = unit_id;
    FS_File_Attr_T file_attr;
    BOOL_T  ret = FALSE;

    memset(&file_attr, 0, sizeof(FS_File_Attr_T));
    if (FS_GetStorageFreeSpace(unit_id, &total_size_of_free_space) != FS_RETURN_OK)
    {
        return FALSE;
    }
    /*delete runtime and not startup first*/
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_TYPE_OPCODE_FILE_TYPE);
    while(FS_GetNextFileInfo(&now_unit,&file_attr) == FS_RETURN_OK)
    {
        if (now_unit != unit_id)
        {
            break;
        }
        if (file_attr.startup_file == FALSE)
        {
            FS_DeleteFile(now_unit, file_attr.file_name);
            FS_GetStorageFreeSpace(unit_id, &total_size_of_free_space);
            if (total_size_of_free_space > file_size)
            {
                ret = TRUE;
                break;
            }
        }
    }
    /*delete config and not startup first*/
    memset(&file_attr, 0, sizeof(FS_File_Attr_T));
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_FILE_TYPE_CONFIG);
    while(FS_GetNextFileInfo(&now_unit,&file_attr) == FS_RETURN_OK)
    {
        if (ret == TRUE)
        {
            break;
        }
        if (now_unit != unit_id)
        {
            break;
        }
        if (file_attr.startup_file == FALSE)
        {
            FS_DeleteFile(now_unit, file_attr.file_name);
            FS_GetStorageFreeSpace(unit_id, &total_size_of_free_space);
            if (total_size_of_free_space > file_size)
            {
                ret = TRUE;
                break;
            }
        }
    }
    return ret;
}



/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Check_File_Version
 *------------------------------------------------------------------------
 * FUNCTION: The function is to check all unit file version
 * INPUT   : file_name -> download file name, file_type -> download file type
 * OUTPUT  : *unit_list -> need update unit list (runtime / config(now always update))
 *
 * RETURN  : TRUE (the same) / FALSE (diffenence)
 * NOTE    : none
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_Check_File_Version(UI8_T *unit_list, UI32_T file_type)
{
    UI32_T i = 0;
    UI32_T my_unit = 0;
    UI8_T  my_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};
    UI8_T  unit_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};
    BOOL_T ret = TRUE;

    STKTPLG_POM_GetMyUnitID(&my_unit);
/* sync $stacking_DB at system startup time is not necessary, since unit table has been
 * moved to CLI config file and STKTPLG will no longer create this file.
 */
#if 0
    /*for unit id table*/
    if (FS_FILE_TYPE_PRIVATE == file_type)
    {
        while (STKTPLG_POM_GetNextUnit(&i))
        {
            if (i == my_unit)/*exclude master*/
            {
                continue;
            }
            unit_list[i - 1] = 1;
        }
        ret = FALSE;
        return ret;
    }
#endif /* end of #if 0*/

    if (FS_FILE_TYPE_CONFIG == file_type)
    {
        /* 2004-04-14 Erica Li
         * Zhong: always transfer to slave (checking size and file name is not good)
         */
        while (STKTPLG_POM_GetNextUnit(&i))
        {
            if (i == my_unit)/*exclude master*/
            {
                continue;
            }
            unit_list[i - 1] = 1;
        }
        ret = FALSE;
        return ret;
    }

    if (   FS_TYPE_OPCODE_FILE_TYPE == file_type
#if (SYS_CPNT_TFTP_DOWNLOAD_LOADER_DIAG == TRUE)
        || FS_FILE_TYPE_DIAG == file_type
#endif
       )
    {
        if (!XFER_MGR_Get_SWFile_Version(my_unit, my_version, file_type))
        {
            return TRUE;
        }

        for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
        {
            if (STKTPLG_POM_UnitExist(i) == FALSE)
            {
                continue;
            }
            if (i == my_unit)/*exclude master*/
            {
                continue;
            }

            memset(unit_version, 0, sizeof(unit_version));

            if (XFER_MGR_Get_SWFile_Version(i, unit_version, file_type))
            {
                if (strcmp((char *)my_version, (char *)unit_version) != 0)
                {
                    unit_list[i - 1] = 1;
                    ret = FALSE;
                }
                else
                {
                    auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_SUCCESS;
                    auto_download_unit[i-1].copy_action = VAL_fileCopyAction_notCopying;
                }
            }
        }
    }
    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Check_File_Version_By_MText
 *------------------------------------------------------------------------
 * FUNCTION: The function is to check all unit file version
 * INPUT   : file_name -> download file name, file_type -> download file type
 * OUTPUT  : *unit_list -> need update unit list (runtime / config(now always update))
 *
 * RETURN  : TRUE (the same) / FALSE (diffenence)
 * NOTE    : none
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_Check_File_Version_By_MText(UI8_T *unit_list, Mtext_T *mtext)
{
    UI32_T      i = 0;
    UI32_T      my_unit = 0;
    UI8_T       my_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};
    UI8_T       unit_version[SYS_ADPT_FW_VER_STR_LEN + 1] = {0};
    BOOL_T      ret = TRUE;
#if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
    image_header_t *imghdr;
#endif

    STKTPLG_POM_GetMyUnitID(&my_unit);
    if (FS_FILE_TYPE_CONFIG == mtext->file_type)
    {
        /* 2004-04-14 Erica Li
         * Zhong: always transfer to slave (checking size and file name is not good)
         */
        while (STKTPLG_POM_GetNextUnit(&i))
        {
            if (i == my_unit) /* exclude master */
            {
                continue;
            }
            unit_list[i - 1] = 1;
        }
        ret = FALSE;
        return ret;
    }

    if (   FS_TYPE_OPCODE_FILE_TYPE == mtext->file_type
#if (SYS_CPNT_TFTP_DOWNLOAD_LOADER_DIAG == TRUE)
        || FS_FILE_TYPE_DIAG == mtext->file_type
#endif
       )
    {
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
        if (XFER_MGR_GetSWVersionFromONIEInstallerStream(mtext->buf, my_version)==FALSE)
        {
            return FALSE;
        }
#else
        imghdr = (image_header_t *)mtext->buf;
        memcpy(my_version, imghdr->ih_ver, SYS_ADPT_FW_VER_STR_LEN+1);
#endif

        for (i = 1; i <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
        {
            if (STKTPLG_POM_UnitExist(i) == FALSE)
            {
                continue;
            }
            if (i == my_unit) /* exclude master */
            {
                continue;
            }
            memset(unit_version, 0, sizeof(unit_version));
            if (XFER_MGR_Get_SWFile_Version(i, unit_version, mtext->file_type))
            {
                if (strcmp((char *)my_version, (char *)unit_version) != 0)
                {
                    unit_list[i - 1] = 1;
                    ret = FALSE;
                }
                else
                {
                    auto_download_unit[i-1].auto_download_status = XFER_MGR_FILE_COPY_SUCCESS;
                    auto_download_unit[i-1].copy_action = VAL_fileCopyAction_notCopying;
                }
            }
        }
    }
    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Get_SWFile_Version
 *------------------------------------------------------------------------
 * FUNCTION: The function is to get startup file version
 * INPUT   : unit_id -> unit id, file_type -> file type(reserve for config)
 * OUTPUT  : *version -> startup file version
 * RETURN  : TRUE (success) / FALSE (fail)
 * NOTE    : none
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_Get_SWFile_Version(UI32_T unit_id, UI8_T *version, UI32_T file_type)
{
    UI8_T           file_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME+1];
#if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
    image_header_t  imghdr;
#else
    UI8_T           *img_header_buf_p;
#endif

    if(FS_GetStartupFilename(unit_id, file_type, file_name) != FS_RETURN_OK)
    {
        printf("XFER: ERROR! Can't get startup runtime image name  on Unit %lu.\r\n", (unsigned long)unit_id);
        return FALSE;
    }

#if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
    if (FS_RETURN_OK != FS_CopyFileContent(unit_id, file_name, 0, (UI8_T *)&imghdr, sizeof(image_header_t)))
    {
        printf("XFER: ERROR! Can't get startup runtime image header on Unit %lu.\r\n", (unsigned long)unit_id);
        return FALSE;
    }

    if(imghdr.ih_magic != IH_MAGIC)
    {
        return FALSE;
    }
    memcpy(version, imghdr.ih_ver, SYS_ADPT_FW_VER_STR_LEN+1);
#else /* #if (SYS_CPNT_ONIE_SUPPORT!=TRUE) */
    img_header_buf_p=L_MM_Malloc(AOS_ONIE_INSTALLER_MAX_IMG_HEADER_SIZE, L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_GETSWFILEVERSION));
    if (img_header_buf_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)L_MM_Malloc failed for %d bytes\r\n", __FUNCTION__, __LINE__,
            AOS_ONIE_INSTALLER_MAX_IMG_HEADER_SIZE);
        return FALSE;
    }
    if (FS_RETURN_OK != FS_CopyFileContent(unit_id, file_name, 0, img_header_buf_p, AOS_ONIE_INSTALLER_MAX_IMG_HEADER_SIZE))
    {
        BACKDOOR_MGR_Printf("%s(%d)Can't get startup runtime image header on Unit %lu.\r\n", __FUNCTION__, __LINE__, (unsigned long)unit_id);
        L_MM_Free(img_header_buf_p);
        return FALSE;
    }
    if (XFER_MGR_GetSWVersionFromONIEInstallerStream(img_header_buf_p, version)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Can't get startup runtime image header on Unit %lu.\r\n", __FUNCTION__, __LINE__, (unsigned long)unit_id);
        L_MM_Free(img_header_buf_p);
        return FALSE;
    }
    L_MM_Free(img_header_buf_p);
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT!=TRUE) */

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_Delete_NonStartOPcode
 *------------------------------------------------------------------------
 * FUNCTION: The function is delete non startup opcode
 * INPUT   : unit_id -> unid_id, file_type -> file type
 * OUTPUT  :
 * RETURN  : none
 * NOTE    :
 *          If have 2 opcode delete non startup file first. This is becuase
 *          file name may not the same as download file, and we only have 2
 *          opcode if only check size and write flash directly may have error
 *------------------------------------------------------------------------*/
static void XFER_MGR_Delete_NonStartOPcode(UI32_T unit_id, UI8_T *dest_file_name_p, UI32_T file_type)
{
    FS_File_Attr_T file_attr;
    UI32_T         opcode_count = 0;
    UI32_T         now_unit = unit_id;
    UI8_T          file_name[XFER_TYPE_MAX_SIZE_OF_LOCAL_FILE_NAME+1] = {0};

    memset(&file_attr, 0, sizeof(FS_File_Attr_T));
    if (file_type != FS_TYPE_OPCODE_FILE_TYPE)
    {
        return;
    }
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_TYPE_OPCODE_FILE_TYPE);
    while(FS_GetNextFileInfo(&now_unit,&file_attr) == FS_RETURN_OK)
    {
        if (now_unit != unit_id)
        {
            break;
        }
        if (file_attr.file_type == FS_TYPE_OPCODE_FILE_TYPE)
        {   /*if dest file name is the same as exist opcode file, do not delete file*/
            if (strcmp((char *)file_attr.file_name, (char *)dest_file_name_p) == 0)
            {
                break;
            }
            if (file_attr.startup_file == FALSE)
            {
                strcpy((char *)file_name, (char *)file_attr.file_name);
            }
            opcode_count++;
        }
        if (opcode_count == 2)
        {
            if (file_attr.startup_file == TRUE)
            {
                FS_DeleteFile(now_unit, file_name);
            }
            else
            {
                FS_DeleteFile(now_unit, file_attr.file_name);
            }
        }
    }
    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CheckImageHeaderValidation
 *------------------------------------------------------------------------
 * FUNCTION: check whether the image header is valid
 * INPUT   : Mtext_T *mtext
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_CheckImageHeaderValidation (Mtext_T *mtext, UI32_T data_len)
{
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    char* tmpfilename_p;
    char* shell_cmd_buf_p;
    int shell_cmd_buf_size;
    UI32_T tmpfilename_size;
    BOOL_T ret_val=TRUE;

    /* Create a temporarily file to do validation through the script file
     */
    if (FS_GetRequiredTmpFilenameBufSize(XFER_MGR_CSC_NAME, &tmpfilename_size)!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_GetRequiredTmpFilenameBufSize returns error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    tmpfilename_p=L_MM_Malloc(tmpfilename_size, L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_CHECKIMAGEHEADERVALIDATION));

    if (tmpfilename_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)L_MM_Malloc failed for %lu bytes.\r\n", __FUNCTION__, __LINE__, (unsigned long)tmpfilename_size);
        return FALSE;
    }

    if (FS_CreateTmpFile(XFER_MGR_CSC_NAME, mtext->buf, data_len,
        tmpfilename_size, tmpfilename_p)!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)FS_CreateTmpFile error\r\n", __FUNCTION__, __LINE__);
        L_MM_Free(tmpfilename_p);
        return FALSE;
    }

    /* ONIE installer is responsible for the validations that is done in this
     * function originally. Invoke the script to do the validation.
     */
    shell_cmd_buf_size=(sizeof(AOS_ONIE_VERIFY_INSTALLER_SCRIPT_FILENAME)-1) +
        1 + /* one space */
        strlen(tmpfilename_p) +
        1; /* terminating null char */
    if (onie_installer_debug_flag==FALSE)
    {
        shell_cmd_buf_size+= 1 /* one space */ +
            sizeof(REDIRECT_STDOUT_STDERR_TO_NULL_CMD)-1; /* "-1" for terminating null char */
    }

    shell_cmd_buf_p=L_MM_Malloc(shell_cmd_buf_size, L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_CHECKIMAGEHEADERVALIDATION));
    if (shell_cmd_buf_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)L_MM_Malloc failed for %d bytes\r\n", __FUNCTION__, __LINE__,
            shell_cmd_buf_size);
        if(FS_RemoveTmpFile(tmpfilename_p)!=FS_RETURN_OK)
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to remove temp file '%s'\r\n", __FUNCTION__,
                __LINE__, tmpfilename_p);
        }
        L_MM_Free(tmpfilename_p);

        return FALSE;
    }
    if (onie_installer_debug_flag==TRUE)
    {
        snprintf(shell_cmd_buf_p, shell_cmd_buf_size, "%s %s",
            AOS_ONIE_VERIFY_INSTALLER_SCRIPT_FILENAME, tmpfilename_p);
        BACKDOOR_MGR_Printf("%s:shell_cmd_buf_p='%s'\r\n", __FUNCTION__, shell_cmd_buf_p);
    }
    else
    {
        snprintf(shell_cmd_buf_p, shell_cmd_buf_size, "%s %s %s",
            AOS_ONIE_VERIFY_INSTALLER_SCRIPT_FILENAME, tmpfilename_p, REDIRECT_STDOUT_STDERR_TO_NULL_CMD);
    }

    if (SYSFUN_ExecuteSystemShell(shell_cmd_buf_p)!=SYSFUN_OK)
    {
        ret_val=FALSE;
    }

    if (FS_RemoveTmpFile(tmpfilename_p)!=FS_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to remove temp file '%s'\r\n", __FUNCTION__,
            __LINE__, tmpfilename_p);
    }
    L_MM_Free(tmpfilename_p);
    L_MM_Free(shell_cmd_buf_p);

    if (ret_val==FALSE)
    {
        /* Simply use header checksum error to represent all types of the error.
         * To Do: This can be enhanced to have more detailed info about the
         *        reason of error.
         */
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        TftpMgrEntry.tftpStatus = XFER_MGR_HEADER_CHECKSUM_ERROR;
        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
    }

    return ret_val;
#else /* #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

    /* Check header checksum
     */
    if (FALSE == FS_CheckImageHeaderCrc(mtext->buf, data_len))
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        TftpMgrEntry.tftpStatus = XFER_MGR_HEADER_CHECKSUM_ERROR;
        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        return FALSE;
    }

    /* Check image checksum
     */
    if (FALSE == FS_CheckImageDataCrc(mtext->buf, data_len))
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_IMAGE_CHECKSUM_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        TftpMgrEntry.tftpStatus = XFER_MGR_IMAGE_CHECKSUM_ERROR;
        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_IMAGE_CHECKSUM_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
        return FALSE;
    }

    if( XFER_MGR_IsCheckProjectId() == TRUE)
    {
        if (FALSE == FS_CheckImageHeaderProductId(mtext->buf, data_len))
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_PROJECT_ID_ERROR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            TftpMgrEntry.tftpStatus = XFER_MGR_IMAGE_TYPE_ERROR;
            TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_PROJECT_ID_ERROR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            return FALSE;
        }
    }
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */
    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SetStartupFilename
 * ------------------------------------------------------------------------
 * FUNCTION : This function will set the startup file base on specified
 *            type and filename, and sync master's startup file to all
 *            slave.
 * INPUT    : file_type   -- startup file type
 *                           (FS_FILE_TYPE_CONFIG / FS_TYPE_OPCODE_FILE_TYPE)
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
                                   void     (*callback) (void *cookie, UI32_T status))
{
    /* DUMMY_DRIVE = Master
     */
    if ( FS_RETURN_OK != FS_SetStartupFilename(DUMMY_DRIVE, file_type, file_name) )
    {
        return FALSE;
    }

#if (SYS_CPNT_XFER_SET_STARTUP_ONLY_FOR_MASTER == TRUE)
    return TRUE;
#else
    /* 1) sync to all unit exclude master
     * 2) auto sync startup is only for config and runtime file
     * 3) the slave's startup file name is the same as master's
     */
    if (file_type == FS_TYPE_OPCODE_FILE_TYPE || file_type == FS_FILE_TYPE_CONFIG)
    {
        return XFER_MGR_AutoDownLoad((UI8_T *)"", file_name, file_name, file_type, TRUE, cookie, ipc_message_q, callback);
    }
    else
    {
        return FALSE;
    }
#endif
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_IsValidServerIP
 * ------------------------------------------------------------------------
 * FUNCTION : This function will check vaildation of server ip.
 * INPUT    : server_p   -- server ip address
 * OUTPUT   : None
 * RETURN   : TRUE        -- ip is valid for server.
 *            FALSE       -- ip is invalid or reserved.
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_IsValidServerIP(
    L_INET_AddrIp_T *server_p)
{
    switch(server_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
        {
            if (IP_LIB_OK != IP_LIB_IsValidForRemoteIp(server_p->addr))
            {
                return FALSE;
            }
        }
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        {
            /* IPv6 address should not be
             * IP_LIB_INVALID_IPV6_UNSPECIFIED
             * IP_LIB_INVALID_IPV6_LOOPBACK
             * IP_LIB_INVALID_IPV6_MULTICAST
             */
            if( IP_LIB_OK != IP_LIB_CheckIPv6PrefixForInterface(server_p->addr, SYS_ADPT_IPV6_ADDR_LEN) )
            {
                return FALSE;
            }
        }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

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
 * NOTE     : 1. if file_type is FS_TYPE_OPCODE_FILE_TYPE, will check whether
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
    void (*callback) (void *cookie, UI32_T status))
{
    XFER_Msg_T   msg;
    Mtext_T *mtext;
    BOOL_T  set_is_busy_flag = FALSE;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    if(dest_file_name == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_FILENAME_NOT_NULL, SYSLOG_LEVEL_INFO,dest_file_name);
        return FALSE;
    }

#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
    if(FS_FILE_TYPE_TOTAL != file_type)
#endif  /* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */
    {
        if (strlen((char *)dest_file_name) > XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
            EH_TYPE_MSG_FILENAME_TOO_LONG, SYSLOG_LEVEL_INFO,dest_file_name);
            return FALSE;
        }

        if (FS_RETURN_OK != FS_GenericFilenameCheck(dest_file_name, file_type))
        {
            return FALSE;
        }
    }

    if (x_buf == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if ( !XFER_MGR_Transaction_Begin(TRUE, &set_is_busy_flag) )
    {
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_FILE_XFER_BUSY, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    if ((mtext = L_MM_Malloc(sizeof(Mtext_T), L_MM_USER_ID2(SYS_MODULE_XFER, XFER_TYPE_TRACE_ID_XFER_MGR_WRITEFILE))) == NULL)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        EH_MGR_Handle_Exception(SYS_MODULE_XFER, XFER_MGR_COPYFILE_FUNC_NO,
        EH_TYPE_MSG_MEM_ALLOCATE, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    /* Copy the arguments passed from UI into the XFER's message. */
    memcpy(&(mtext->user_info), user_info_p, sizeof(mtext->user_info));
    mtext->buf          = x_buf;
    mtext->length       = length;
    mtext->cookie       = cookie;
    mtext->callback     = callback;
    mtext->file_type    = file_type;
    mtext->is_next_boot = info.is_next_boot;
    mtext->ipc_message_q= ipc_message_q;
    mtext->src_oper_type = VAL_fileCopySrcOperType_http;
    mtext->dest_oper_type = VAL_fileCopyDestOperType_file;
    strcpy((char *)mtext->destfile, (char *)dest_file_name);

    /*Hans add for Web configuration upload/download*/
    if(mtext->file_type == FS_TYPE_OPCODE_FILE_TYPE)
    {

        /* Check the image header and verify the image checksum. */
        if (FALSE == XFER_MGR_CheckImageHeaderValidation(mtext,length))
        {
            XFER_MGR_Transaction_End(set_is_busy_flag);
            BUFFER_MGR_Free(mtext->buf);
            L_MM_Free(mtext);
            return FALSE;
        }
    }
#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
    else if(mtext->file_type == FS_FILE_TYPE_TOTAL)
    {
        if (FALSE == XFER_MGR_CheckLoaderImageValidation(mtext))
        {
            XFER_MGR_Transaction_End(set_is_busy_flag);
            BUFFER_MGR_Free(mtext->buf);
            L_MM_Free(mtext);
            return FALSE;
        }
    }
#endif  /* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */

    msg.mtype = XFER_MGR_WRITEFILE;
    msg.mtext = mtext;

    if (is_file_copy_mgt_used == FALSE)
    {
        XFER_MGR_SetFileCopySrcOperType(VAL_fileCopySrcOperType_http);
        XFER_MGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_file);
        XFER_MGR_SetFileCopyDestFileName(dest_file_name);

        file_copy_mgt_entry.file_type = file_type;
        file_copy_mgt_entry.action    = VAL_fileCopyAction_copy;
    }

    memset(file_copy_mgt_entry.tftp_error_msg, 0, sizeof(file_copy_mgt_entry.tftp_error_msg));


    UI8_T msg_space_p[SYSFUN_SIZE_OF_MSG(sizeof(XFER_Msg_T))];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;
	req_msg_p->msg_type = 1;
    req_msg_p->msg_size= sizeof(XFER_Msg_T);
    memcpy(req_msg_p->msg_buf,&msg,sizeof(XFER_Msg_T));

    if(SYSFUN_SendRequestMsg(xfer_mgr_task_msgQ_id, req_msg_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, XFER_MGR_EVENT_MSG, 0, NULL) != SYSFUN_OK)
    {
        XFER_MGR_Transaction_End(set_is_busy_flag);
        BUFFER_MGR_Free(mtext->buf);
        L_MM_Free(mtext);
        XFER_MGR_DUMP_FAIL_REASON;
        /*  log to system : send event to task -- fail  */
        DBG_PrintText (" XFER_TASK : Send event to XFER_TASK fail..\n");
        return FALSE;
    }

    return  TRUE;

} /* End of XFER_MGR_WriteFile() */


#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CheckLoaderImageValidation
 * ------------------------------------------------------------------------
 * FUNCTION : This function will be used to copy the remote loader file.
 * INPUT    : mtext   --
 * OUTPUT   : None
 * RETURN   : TRUE    -- the image is valid
 *            FALSE   -- the image is illegal.
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static BOOL_T XFER_MGR_CheckLoaderImageValidation (Mtext_T *mtext)
{
    LDR_INFO_BLOCK_T    *loader_img_hdr = NULL;

    loader_img_hdr = (LDR_INFO_BLOCK_T *)(mtext->buf);

    /* Original, loader_img_hdr->identifier should always be "LDR0", which
     * is LDR_INFO_BLOCK_ID. However, the 3rd byte and 4th bytes of
     * loader_img_hdr had been stolen to represent project id.
     * See UPGRADE_loader() at uboot/common/upgrade.c for details.
     * 2009/10/28 charlie_chen
     */
    if (0 != strncmp((char *)loader_img_hdr->identifier, LDR_INFO_BLOCK_ID, sizeof((char *)loader_img_hdr->identifier)-2))
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_IMAGE_TYPE_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        TftpMgrEntry.tftpStatus = XFER_MGR_ERROR;
        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_IMAGE_TYPE_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;

        return FALSE;
    }

#if (SYS_CPNT_ONIE_SUPPORT!=TRUE)
    /* Check project id
     */
    if( XFER_MGR_IsCheckProjectId() == TRUE)
    {
        if(FS_CheckLoaderImageHeaderProductId(loader_img_hdr)==FALSE)
        {
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_PROJECT_ID_ERROR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
            XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
            TftpMgrEntry.tftpStatus = XFER_MGR_IMAGE_TYPE_ERROR;
            TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
            file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_PROJECT_ID_ERROR;
            file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;
            return FALSE;
        }
    }
#endif

    /* Check block checksum
     */
    if (loader_img_hdr->block_checksum != L_MATH_CheckSum((char *)loader_img_hdr, sizeof(LDR_INFO_BLOCK_T) - sizeof(loader_img_hdr->block_checksum)))
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        TftpMgrEntry.tftpStatus = XFER_MGR_HEADER_CHECKSUM_ERROR;
        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;

        return FALSE;
    }

    /* Check image checksum
     */
    if (loader_img_hdr->checksum != L_MATH_CheckSum((char *)mtext->buf + sizeof(LDR_INFO_BLOCK_T), loader_img_hdr->length))
    {
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_IMAGE_CHECKSUM_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_ERROR);
        XFER_MGR_CallBack(mtext, XFER_MGR_FILE_COPY_COMPLETED);
        TftpMgrEntry.tftpStatus = XFER_MGR_IMAGE_CHECKSUM_ERROR;
        TftpMgrEntry.tftpAction = VAL_tftpAction_notDownloading;
        file_copy_mgt_entry.status = XFER_MGR_FILE_COPY_IMAGE_CHECKSUM_ERROR;
        file_copy_mgt_entry.action = VAL_fileCopyAction_notCopying;

        return FALSE;
    }

    return TRUE;
}

#endif /* end of (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_CopyUserInfoByServerType
 * ------------------------------------------------------------------------
 * FUNCTION : Copy user info (username and password) according to server type.
 * INPUT    : server_type
 *            src_username      -- source of username
 *            src_password      -- source of password
 * OUTPUT   : dst_username      -- destination of username
 *            dst_password      -- destination of password
 * RETURN   : TRUE/FALSE
 * NOTE     : If server doesn't require username and password (ex: TFTP),
 *            it will not copy any username or password.
 * ------------------------------------------------------------------------
 */
static BOOL_T XFER_MGR_CopyUserInfoByServerType(XFER_MGR_RemoteServer_T server_type, const UI8_T *src_username, const UI8_T *src_password, UI8_T *dst_username, UI8_T *dst_password)
{
    switch (server_type)
    {
        case XFER_MGR_REMOTE_SERVER_TFTP:
            return FALSE;

        case XFER_MGR_REMOTE_SERVER_FTP:
            break;

        case XFER_MGR_REMOTE_SERVER_FTPS:
            break;

#if (SYS_CPNT_SFTP == TRUE)
        case XFER_MGR_REMOTE_SERVER_SFTP:
            break;
#endif /* #if (SYS_CPNT_SFTP == TRUE) */

        default:
            return FALSE;
    }

    if (   (NULL == src_username)
        || (NULL == src_password)
        || (NULL == dst_username)
        || (NULL == dst_password))
    {
        return FALSE;
    }

    memset(dst_username, 0, MAXSIZE_fileCopyServerUserName+1);
    if ('\0' == *src_username)
    {
        strncpy((char *)dst_username, XFER_MGR_DFLT_USERNAME, MAXSIZE_fileCopyServerUserName);
    }
    else
    {
        strncpy((char *)dst_username, (char *)src_username, MAXSIZE_fileCopyServerUserName);
    }

    memset(dst_password, 0, MAXSIZE_fileCopyServerPassword+1);
    if ('\0' == *src_username)
    {
        strncpy((char *)dst_password, XFER_MGR_DFLT_PASSWORD, MAXSIZE_fileCopyServerPassword);
    }
    else
    {
        strncpy((char *)dst_password, (char *)src_password, MAXSIZE_fileCopyServerPassword);
    }

    return TRUE;
}

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ParseUrl
 *------------------------------------------------------------------------
 * FUNCTION: Brute-force parsing of an ftp: URL string.
 * INPUT   : src   - URL string to be parsed.  Note that the string itself
 *                   will be edited.  If <src> is NULL, this function will
 *                   initialize <list> as an empty list.
 * OUTPUT  : list  - An array of pointers of type (char *).  These pointers
 *                   will point to the parsed subfields within the <src>
 *                   string.
 * RETURN  : TRUE/FALSE
 * Notes   : The contents of <src> are modified by this function.  If you
 *           need an intact copy of the original URL string, use strdup()
 *           to make a copy and pass the copy.  Do not forget to free the
 *           new string's memory when you are finished using the parsed
 *           URL.
 *
 *           This function only chops the URL into pieces.  It does not
 *           validate the content of those pieces, nor does it translate
 *           any URL escapes.  Escape sequences should be translated
 *           *after* the string is parsed.
 *
 * Usage   : url <URL>, where <URL> is formatted roughly as follows:
 *
 *  scheme://[[user[:passwd]@]host[:port][/path]]
 *
 *-----------------------------------------------------------------------------
 */
static BOOL_T XFER_MGR_ParseUrl(char *src, char *list[XFER_MGR_URL_TOKEN_MAX])
{
    enum {MAX_LENGTH_OF_TEMP_STRING = 6};

    int  i;
    char *p;
    char str_temp[MAX_LENGTH_OF_TEMP_STRING + 1];

    /* Initialize the parse field list.
     */
    for( i = 0; i < XFER_MGR_URL_TOKEN_MAX; i++ )
        list[i] = NULL;

    /* It's okay if the source is NULL.
     * We just can't parse anything if it is.
     */
    if (NULL == src)
        return TRUE;

    i = (int)strcspn( src, ":" );
    if( ':' == src[i] )
    {
        src[i] = '\0';
        list[XFER_MGR_URL_TOKEN_SCHEME] = src;
        src += (i+1);
    }

    /* If there have a double slashes ("//"), the following characters before
     * next slashe ("/") should be authority.
     */
    strncpy(str_temp, src, 2);
    str_temp[2] = 0;
    if (0 == strcmp(str_temp, "//"))
    {
        int at_symbol_index;

        src += 2;

        i = (int)strcspn( src, "/" );
        if( '/' == src[i] )
        {
            src[i] = '\0';  /* Terminate the first half: the authority string. */
            i++;            /* <i> marks the start of the path string.   */

            /* Parse the path/ string first.  It starts at src[i]. */
            if( '\0' != src[i] )  /* The pathname string exists. */
            {
                list[XFER_MGR_URL_TOKEN_PATHNAME] = &(src[i]);
            }
        }

        /* All that should be left of src is the authority string.
         * That is, the optional user info, the host name/IP, and
         * optional port number:
         *
         *      [[user[:password]@]host[:port]]
         *
         * Start by separating the host name from the user
         * information.
         */
        at_symbol_index = (int)strcspn( src, "@" );
        if( '@' == src[at_symbol_index] )
        {
            int colon_symbol_index;

            if (0 == at_symbol_index)
        {
                /* Not accept empty username.
                 */
                return FALSE;
            }

            /* Got auth info. */
            list[XFER_MGR_URL_TOKEN_HOST] = &(src[at_symbol_index + 1]);  /* Host name follows user info. */
            src[at_symbol_index] = '\0';

            /* Okay, we have the auth string:  user[:password]
             * Pull out ntdomain, user, and password.  Password first.
             */
            colon_symbol_index = (int)strcspn(src, ":" );
            if( ':' == src[colon_symbol_index] )
            {
                if (   (0 == colon_symbol_index)
                    || (at_symbol_index == (colon_symbol_index + 1))
                    )
                {
                    /* Not accept empty username or password.
                     */
                    return FALSE;
                }

                list[XFER_MGR_URL_TOKEN_PASSWORD] = &(src[colon_symbol_index + 1]);
                src[colon_symbol_index] = '\0';
            }

            list[XFER_MGR_URL_TOKEN_USER] = src;
        }
        else
        {
            /* If we found no '@', then there is no user info.
             */
            list[XFER_MGR_URL_TOKEN_HOST] = src;
        }
    }
    else
    {
        /* Removes the first slashe ("/") from the pathname.
         */
        if ('/' == *src)
            src += 1;

        list[XFER_MGR_URL_TOKEN_PATHNAME] = src;
    }

    /* Final bit is to separate the port number (if any) from the host name.
     * Thing is, the separator is a colon (':').  The colon may also exist
     * in the host portion if the host is specified as an IPv6 address (see
     * RFC 2732).  If that's the case, then we need to skip past the IPv6
     * address, which should be contained within square brackets ('[',']').
     */
    if (NULL != list[XFER_MGR_URL_TOKEN_HOST])
    {
        p = strchr( list[XFER_MGR_URL_TOKEN_HOST], '[' );   /* Look for '['.            */
        if( NULL != p )                                     /* If found, look for ']'.  */
            p = strchr( p, ']' );
        if( p == NULL )                                     /* If '['..']' not found,   */
            p = list[XFER_MGR_URL_TOKEN_HOST];              /* scan the whole string.   */

        /* Starting at <p>, which is either the start of the host substring
         * or the end of the IPv6 address, find the last colon character.
         */
        p = strchr( p, ':' );
        if( NULL != p )
        {
            *p = '\0';
            list[XFER_MGR_URL_TOKEN_PORT] = p + 1;
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_IsValidUrl
 *------------------------------------------------------------------------
 * FUNCTION: Validates an URL string for auto upgrade
 * INPUT   : src   - URL string to be validated
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * Notes   :
 *
 * Usage   : url <URL>, where <URL> is formatted roughly as follows:
 *
 *  scheme://[user[:passwd]@]host[:port][/path]/
 *
 * The scheme and host MUST be included in the URL string. The last character
 * MUST be a slash sign
 *
 * No support IPv6 address and domain name format for host part
 *
 *-----------------------------------------------------------------------------
 */
static BOOL_T XFER_MGR_IsValidUrl(const char *src)
{
    char   *list[XFER_MGR_URL_TOKEN_MAX];
    char   *src_cp_p;
    int    len;
    BOOL_T ret;

    /* Check null pointer
     */
    if (NULL == src)
    {
        return FALSE;
    }

    /* The last character MUST be a slash sign.
     */
    len = strlen(src);
    if (len > 1)
    {
        if ('/' != src[len-1])
        {
            return FALSE;
        }
    }

    src_cp_p = (char *)malloc(len + 1);
    strcpy(src_cp_p, src);
    src_cp_p[len] = '\0';

    ret = XFER_MGR_ParseUrl(src_cp_p, list);
    if (TRUE == ret)
    {
        if (NULL == list[XFER_MGR_URL_TOKEN_SCHEME] || NULL == list[XFER_MGR_URL_TOKEN_HOST])
        {
            ret = FALSE;
        }
    }

    if (TRUE == ret)
    {
        L_STDLIB_StringN_To_Lower((UI8_T *)list[XFER_MGR_URL_TOKEN_SCHEME], strlen(list[XFER_MGR_URL_TOKEN_SCHEME]));

        if (    (0 != strcmp(list[XFER_MGR_URL_TOKEN_SCHEME], "tftp"))
    #if (SYS_CPNT_XFER_FTP == TRUE)
            &&  (0 != strcmp(list[XFER_MGR_URL_TOKEN_SCHEME], "ftp"))
    #endif
           )
        {
            ret = FALSE;
        }
    }

    /* Check length whether over.
     */
#if (SYS_CPNT_XFER_FTP == TRUE)
    if ((TRUE == ret) && (NULL != list[XFER_MGR_URL_TOKEN_USER]))
    {
        if (strlen(list[XFER_MGR_URL_TOKEN_USER]) > MAXSIZE_fileCopyFTPLoginUsername)
        {
            ret = FALSE;
        }
    }

    if ((TRUE == ret) && (NULL != list[XFER_MGR_URL_TOKEN_PASSWORD]))
    {
        if (strlen(list[XFER_MGR_URL_TOKEN_PASSWORD]) > MAXSIZE_fileCopyFTPLoginPassword)
        {
            ret = FALSE;
        }
    }
#endif

    if ((TRUE == ret) && (NULL != list[XFER_MGR_URL_TOKEN_HOST]))
    {
        L_INET_AddrIp_T ip_address;

        memset(&ip_address, 0, sizeof(ip_address));

        /* no support ipv6 and domain name format
         */
        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                           list[XFER_MGR_URL_TOKEN_HOST],
                                                           (L_INET_Addr_T *)&ip_address,
                                                           sizeof(ip_address)))
        {
            ret = FALSE;
        }
    }

    if ((TRUE == ret) && (NULL != list[XFER_MGR_URL_TOKEN_PATHNAME]))
    {
        if ((1+strlen(list[XFER_MGR_URL_TOKEN_PATHNAME])+strlen(SYS_ADPT_XFER_AUTO_UPGRADE_OPCODE_SEARCH_FILENAME)) > XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME)
        {
            ret = FALSE;
        }
    }

    free(src_cp_p);
    return ret;
}
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XFER_MGR_IsFileNumExceededByType
 *-------------------------------------------------------------------------
 * PURPOSE  : check the number of file type is exceeded.
 * INPUT    : UI8_T *filename, UI32_T file_type
 * OUTPUT   : None
 * RETURN   : TRUE - full; FALSE - not full
 * NOTE     :
 *-------------------------------------------------------------------------*/
static BOOL_T XFER_MGR_IsFileNumExceededByType(UI8_T *filename, UI32_T file_type)
{
    BOOL_T is_exist = FALSE;
    UI32_T unit_id = 0;
    UI32_T max_file_num = 0;
    FS_File_Attr_T  file_attr;

    STKTPLG_POM_GetMyUnitID(&unit_id);
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(file_type);
    strcpy((char *)file_attr.file_name, (char *)filename);

    if(FS_RETURN_OK == FS_GetFileInfo(unit_id, &file_attr))
    {
        is_exist = TRUE;
    }
    else
    {
        is_exist = FALSE;
    }

    if (file_type == FS_TYPE_OPCODE_FILE_TYPE)
    {
        max_file_num = FS_TYPE_MAX_NUM_OF_FILE_OP_CODE;
    }
    else if (file_type == FS_FILE_TYPE_CONFIG)
    {
        max_file_num = SYS_ADPT_MAX_NUM_OF_FILE_CONFIG;
    }

    if (max_file_num == FS_NumOfFile(file_type)&& is_exist != TRUE )
    {
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ServerTypeToSrcOperType
 *------------------------------------------------------------------------
 * FUNCTION: convert server type to source operation type
 * INPUT   : server_type    -- remote server type
 * OUTPUT  : src_oper_type  -- source operation type
 * RETURN  : TRUE / FALSE
 * NOTE    : none
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_ServerTypeToSrcOperType(
    XFER_MGR_RemoteServer_T server_type,
    UI32_T *src_oper_type)
{
    switch (server_type)
    {
        case XFER_MGR_REMOTE_SERVER_TFTP:
            *src_oper_type = VAL_fileCopySrcOperType_tftp;
            break;

        case XFER_MGR_REMOTE_SERVER_FTP:
            *src_oper_type = VAL_fileCopySrcOperType_ftp;
            break;

        case XFER_MGR_REMOTE_SERVER_SFTP:
            *src_oper_type = VAL_fileCopySrcOperType_sftp;
            break;

        case XFER_MGR_REMOTE_SERVER_FTPS:
            *src_oper_type = VAL_fileCopySrcOperType_ftps;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}/* End of XFER_MGR_ServerTypeToSrcOperType */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_ServerTypeToDestOperType
 *------------------------------------------------------------------------
 * FUNCTION: convert server type to destnation operation type
 * INPUT   : server_type    -- remote server type
 * OUTPUT  : dest_oper_type -- destnation operation type
 * RETURN  : TRUE / FALSE
 * NOTE    : none
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_ServerTypeToDestOperType(
    XFER_MGR_RemoteServer_T server_type,
    UI32_T *dest_oper_type)
{
    switch (server_type)
    {
        case XFER_MGR_REMOTE_SERVER_TFTP:
            *dest_oper_type = VAL_fileCopyDestOperType_tftp;
            break;

        case XFER_MGR_REMOTE_SERVER_FTP:
            *dest_oper_type = VAL_fileCopyDestOperType_ftp;
            break;

        case XFER_MGR_REMOTE_SERVER_SFTP:
            *dest_oper_type = VAL_fileCopyDestOperType_sftp;
            break;

        case XFER_MGR_REMOTE_SERVER_FTPS:
            *dest_oper_type = VAL_fileCopyDestOperType_ftps;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}/* End of XFER_MGR_ServerTypeToDestOperType */

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_MGR_SendFileCopyTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send file copy trap
 * INPUT   : mtext - context of message of xfer
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
XFER_MGR_SendFileCopyTrap(
    Mtext_T *mtext)
{
    TRAP_EVENT_TrapData_T  data;

    memset(&data, 0, sizeof(data));
    data.trap_type = TRAP_EVENT_FILE_COPY;
    data.community_specified = FALSE;

    data.u.file_copy_entry.user_info.session_type = mtext->user_info.session_type;
    strncpy(data.u.file_copy_entry.user_info.user_name,
        mtext->user_info.user_name,
        sizeof(data.u.file_copy_entry.user_info.user_name)-1);
    data.u.file_copy_entry.user_info.user_name[sizeof(data.u.file_copy_entry.user_info.user_name)-1] = '\0';
    memcpy(&data.u.file_copy_entry.user_info.user_ip, &mtext->user_info.user_ip,
        sizeof(data.u.file_copy_entry.user_info.user_ip));
    memcpy(data.u.file_copy_entry.user_info.user_mac, mtext->user_info.user_mac,
        sizeof(data.u.file_copy_entry.user_info.user_mac));

    switch (mtext->file_type)
    {
        case FS_FILE_TYPE_DIAG:
            data.u.file_copy_entry.file_copy_info.file_type = VAL_fileCopyFileType_bootRom;
            break;

        case FS_TYPE_OPCODE_FILE_TYPE:
            data.u.file_copy_entry.file_copy_info.file_type = VAL_fileCopyFileType_opcode;
            break;

        case FS_FILE_TYPE_CONFIG:
            data.u.file_copy_entry.file_copy_info.file_type = VAL_fileCopyFileType_config;
            break;

        case FS_FILE_TYPE_CERTIFICATE:
            // FIXME:
            // [PATCH: wrong syslog for copying cert](AOS5600-52X-00408)
            //
            // data.u.file_copy_entry.file_copy_info.file_type = VAL_fileCopyFileType_publickey;
            data.u.file_copy_entry.file_copy_info.file_type = mtext->publickey_username[0] == '\0' ?
                VAL_fileCopyFileType_certificate : VAL_fileCopyFileType_publickey;
            break;

        case FS_FILE_TYPE_TOTAL:
            data.u.file_copy_entry.file_copy_info.file_type = VAL_fileCopyFileType_loader;
            break;

        default:
            return FALSE;
   }

    data.u.file_copy_entry.file_copy_info.src_oper_type = mtext->src_oper_type;
    data.u.file_copy_entry.file_copy_info.dest_oper_type = mtext->dest_oper_type;
    data.u.file_copy_entry.file_copy_info.unit = mtext->unit_id;
    data.u.file_copy_entry.file_copy_info.status = mtext->status;
    strncpy(data.u.file_copy_entry.file_copy_info.src_file_name,
        (char *)mtext->srcfile,
        sizeof(data.u.file_copy_entry.file_copy_info.src_file_name)-1);
    data.u.file_copy_entry.file_copy_info.src_file_name[sizeof(data.u.file_copy_entry.file_copy_info.src_file_name)-1] = '\0';
    strncpy(data.u.file_copy_entry.file_copy_info.dest_file_name,
        (char *)mtext->destfile,
        sizeof(data.u.file_copy_entry.file_copy_info.dest_file_name)-1);
    data.u.file_copy_entry.file_copy_info.dest_file_name[sizeof(data.u.file_copy_entry.file_copy_info.dest_file_name)-1] = '\0';
    memcpy(&data.u.file_copy_entry.file_copy_info.server_address, &mtext->server_ip,
        sizeof(data.u.file_copy_entry.file_copy_info.server_address));

    SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    return TRUE;
}/* End of XFER_MGR_SendFileCopyTrap */

static void
XFER_MGR_WaitForTransmitCompleted()
{
    const UI32_T sleep_ticks = 200;
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    const UI32_T wdog_reset_time = (SYS_ADPT_XFER_SW_WATCHDOG_TIMER * 60 * 100) / 2; /* 0.01 sec */
    UI32_T wdog_timer = wdog_reset_time;
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */

    while (1)
    {
        if (info.current_tftp_status == XFER_DNLD_TFTP_COMPLETED)
        {
            break;
        }
        SYSFUN_Sleep(sleep_ticks);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        wdog_timer += sleep_ticks;
        if (wdog_reset_time <= wdog_timer)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_XFER);
            wdog_timer = 0;
        }
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
    }

    return;
}

static int XFER_MGR_LicenseResHandler(void *output_p, const char *section_p, const char *name_p, const char *value_p)
{
    UI32_T *max_valid_version_p = (UI32_T *)output_p;

    if (   (0 == strcmp(section_p, ""))
        && (0 == strcmp(name_p, "max-valid-version"))
        )
    {
        if (TRUE == XFER_MGR_IsDebugFlagOn())
        {
            BACKDOOR_MGR_Printf("%s: max-valid-version=%s\r\n", __FUNCTION__, value_p);
        }

        *max_valid_version_p = atoi(value_p);
    }

    return 1;
}

static BOOL_T XFER_MGR_IsOpcodeVersionAllowByLicense(char *version_p)
{
    enum
    {
        VERSION_PART = 4
    };

    UI32_T max_valid_version = 0xFFFFFFFF;
    UI32_T version_ar[VERSION_PART];

    if (0 > ini_parse(SYS_ADPT_LICENSE_FILE_PATH SYS_ADPT_LICENSE_RESULT_FILE_NAME, XFER_MGR_LicenseResHandler, &max_valid_version))
    {
        if (TRUE == XFER_MGR_IsDebugFlagOn())
        {
            BACKDOOR_MGR_Printf("%s: Failed to read ini file.\r\n", __FUNCTION__);
        }

        return FALSE;
    }

    if (0xFFFFFFFF == max_valid_version)
    {
        /* No limitation if key max-valid-version is not found in ini file.
         */

        if (TRUE == XFER_MGR_IsDebugFlagOn())
        {
            BACKDOOR_MGR_Printf("%s: Key max-valid-version is not found in ini file.\r\n", __FUNCTION__);
        }

        return TRUE;
    }

    if (VERSION_PART != sscanf(version_p, "%u.%u.%u.%u", &version_ar[0], &version_ar[1], &version_ar[2], &version_ar[3]))
    {
        if (TRUE == XFER_MGR_IsDebugFlagOn())
        {
            BACKDOOR_MGR_Printf("%s: Failed to parse new version %s.\r\n", __FUNCTION__, version_p);
        }

        return FALSE;
    }

    if (version_ar[VERSION_PART - 1] > max_valid_version)
    {
        if (TRUE == XFER_MGR_IsDebugFlagOn())
        {
            BACKDOOR_MGR_Printf("%s: Last number in new version (%lu) newer than license ones (%lu) is not permitted.\r\n",
                __FUNCTION__, version_ar[VERSION_PART - 1], max_valid_version);
        }

        return FALSE;
    }

    return TRUE;
}

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
BOOL_T XFER_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    XFER_MGR_IpcMsg_T   *msg_p;

    UI8_T               *buffer_pointer;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (XFER_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (XFER_MGR_GetOperationMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding AMTR_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case XFER_MGR_IPC_STREAMTOLOCAL_4WRITING:
            buffer_pointer =(UI8_T *) BUFFER_MGR_GetPtr(msg_p->data.arg_grp_bool_streamtolocal.arg_buffer_offset);
            msg_p->type.ret_bool = XFER_MGR_StreamToLocal_write(
                &msg_p->data.arg_grp_bool_streamtolocal.arg_user_info,
                msg_p->data.arg_grp_bool_streamtolocal.arg_filename,
                msg_p->data.arg_grp_bool_streamtolocal.arg_file_type,
                msg_p->data.arg_grp_bool_streamtolocal.arg_cookie,
                msg_p->data.arg_grp_bool_streamtolocal.arg_ipc_message_q,
                buffer_pointer,
                msg_p->data.arg_grp_bool_streamtolocal.arg_buffer_length,
                msg_p->data.arg_grp_bool_streamtolocal.arg_buffer_offset,
                msg_p->data.arg_grp_bool_streamtolocal.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_streamtolocal);
            break;
        case XFER_MGR_IPC_COPYFILE:
            msg_p->type.ret_bool = XFER_MGR_CopyFile(
                &msg_p->data.arg_grp_bool_copyfile.arg_user_info,
                msg_p->data.arg_grp_bool_copyfile.arg_publickey_username,
                msg_p->data.arg_grp_bool_copyfile.arg_publickey_type,
                &msg_p->data.arg_grp_bool_copyfile.arg_server_ip,
                msg_p->data.arg_grp_bool_copyfile.arg_destfile,
                msg_p->data.arg_grp_bool_copyfile.arg_srcfile,
                msg_p->data.arg_grp_bool_copyfile.arg_file_type,
                msg_p->data.arg_grp_bool_copyfile.arg_mode,
                msg_p->data.arg_grp_bool_copyfile.arg_server_type,
                msg_p->data.arg_grp_bool_copyfile.arg_username,
                msg_p->data.arg_grp_bool_copyfile.arg_password,
                msg_p->data.arg_grp_bool_copyfile.arg_cookie,
                msg_p->data.arg_grp_bool_copyfile.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_copyfile.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_copyfile);
            break;

        case XFER_MGR_IPC_REMOTETOSTREAM:
            buffer_pointer =(UI8_T *) BUFFER_MGR_GetPtr(msg_p->data.arg_grp_bool_remotetoStream.arg_x_buf_offset);
            msg_p->type.ret_bool = XFER_MGR_RemoteToStream(
                &msg_p->data.arg_grp_bool_remotetoStream.arg_user_info,
                msg_p->data.arg_grp_bool_remotetoStream.arg_server_type,
                &msg_p->data.arg_grp_bool_remotetoStream.arg_server_ip,
                msg_p->data.arg_grp_bool_remotetoStream.arg_username,
                msg_p->data.arg_grp_bool_remotetoStream.arg_password,
                msg_p->data.arg_grp_bool_remotetoStream.arg_srcfile,
                msg_p->data.arg_grp_bool_remotetoStream.arg_file_type,
                buffer_pointer,
                msg_p->data.arg_grp_bool_remotetoStream.arg_cookie,
                msg_p->data.arg_grp_bool_remotetoStream.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_remotetoStream.arg_callback,
                msg_p->data.arg_grp_bool_remotetoStream.arg_stream_max_length);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_remotetoStream);
            break;

        case XFER_MGR_IPC_STREAMTOREMOTE_CHECK:
        	msg_p->type.ret_bool = XFER_MGR_StreamToRemote_Check(
        	    &msg_p->data.arg_grp_bool_streamtoremote.arg_server_ip,
        	    msg_p->data.arg_grp_bool_streamtoremote.arg_desfile,
        	    msg_p->data.arg_grp_bool_streamtoremote.arg_file_type,
        	    msg_p->data.arg_grp_bool_streamtoremote.arg_cookie,
        	    msg_p->data.arg_grp_bool_streamtoremote.arg_ipc_message_q,
        	    msg_p->data.arg_grp_bool_streamtoremote.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_streamtoremote);
            break;

        case XFER_MGR_IPC_STREAMTOREMOTE_WRITING:
            buffer_pointer =(UI8_T *) BUFFER_MGR_GetPtr(msg_p->data.arg_grp_bool_streamtoremote.arg_buffer_offset);
            msg_p->type.ret_bool = XFER_MGR_StreamToRemote_Write(
                &msg_p->data.arg_grp_bool_streamtoremote.arg_user_info,
                msg_p->data.arg_grp_bool_streamtoremote.arg_server_type,
                &msg_p->data.arg_grp_bool_streamtoremote.arg_server_ip,
                msg_p->data.arg_grp_bool_streamtoremote.arg_username,
                msg_p->data.arg_grp_bool_streamtoremote.arg_password,
                msg_p->data.arg_grp_bool_streamtoremote.arg_desfile,
                msg_p->data.arg_grp_bool_streamtoremote.arg_file_type,
                msg_p->data.arg_grp_bool_streamtoremote.arg_cookie,
                msg_p->data.arg_grp_bool_streamtoremote.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_streamtoremote.arg_buffer_length,
                msg_p->data.arg_grp_bool_streamtoremote.arg_buffer_offset,
                msg_p->data.arg_grp_bool_streamtoremote.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_streamtoremote);
            break;

        case XFER_MGR_IPC_STREAMTOREMOTE:
            msg_p->type.ret_bool = XFER_MGR_StreamToRemote(
                &msg_p->data.arg_grp_bool_streamtoremote.arg_user_info,
                msg_p->data.arg_grp_bool_streamtoremote.arg_server_type,
                &msg_p->data.arg_grp_bool_streamtoremote.arg_server_ip,
                msg_p->data.arg_grp_bool_streamtoremote.arg_username,
                msg_p->data.arg_grp_bool_streamtoremote.arg_password,
                msg_p->data.arg_grp_bool_streamtoremote.arg_desfile,
                msg_p->data.arg_grp_bool_streamtoremote.arg_file_type,
                msg_p->data.arg_grp_bool_streamtoremote.arg_cookie,
                msg_p->data.arg_grp_bool_streamtoremote.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_streamtoremote.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_streamtoremote);
            break;

        case XFER_MGR_IPC_STREAMTOLOCAL:
             msg_p->type.ret_bool = XFER_MGR_StreamToLocal(
                &msg_p->data.arg_grp_bool_streamtolocal.arg_user_info,
                msg_p->data.arg_grp_bool_streamtolocal.arg_filename,
                msg_p->data.arg_grp_bool_streamtolocal.arg_file_type,
                msg_p->data.arg_grp_bool_streamtolocal.arg_cookie,
                msg_p->data.arg_grp_bool_streamtolocal.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_streamtolocal.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_streamtolocal);
            break;

        case XFER_MGR_IPC_LOCALTOSTREAM:
            buffer_pointer =(UI8_T *) BUFFER_MGR_GetPtr(msg_p->data.arg_grp_ui32_localtostream.arg_xbuffer_offset);
            msg_p->type.ret_ui32 = XFER_MGR_LocalToStream(
                &msg_p->data.arg_grp_ui32_localtostream.arg_user_info,
                msg_p->data.arg_grp_ui32_localtostream.arg_filename,
                buffer_pointer,
                &msg_p->data.arg_grp_ui32_localtostream.arg_xbuf_length,
                msg_p->data.arg_grp_ui32_localtostream.arg_stream_max_length);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_ui32_localtostream);
            break;

        case XFER_MGR_IPC_READSYSTEMCONFIG:
            buffer_pointer =(UI8_T *) BUFFER_MGR_GetPtr(msg_p->data.arg_grp_ui32_readsystemconfig.arg_xbuffer_offset);
        	msg_p->type.ret_ui32 = XFER_MGR_ReadSystemConfig(
        	    buffer_pointer,
        	    &msg_p->data.arg_grp_ui32_readsystemconfig.arg_xbuf_length);

            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_ui32_readsystemconfig);
            break;

        case XFER_MGR_IPC_UNITTOLOCAL:
            msg_p->type.ret_bool = XFER_MGR_UnitToLocal(
                &msg_p->data.arg_grp_bool_unittolocal.arg_user_info,
                msg_p->data.arg_grp_bool_unittolocal.arg_unit_id,
                msg_p->data.arg_grp_bool_unittolocal.arg_destfile,
                msg_p->data.arg_grp_bool_unittolocal.arg_srcfile,
                msg_p->data.arg_grp_bool_unittolocal.arg_file_type,
                msg_p->data.arg_grp_bool_unittolocal.arg_cookie,
                msg_p->data.arg_grp_bool_unittolocal.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_unittolocal.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_unittolocal);
            break;

        case XFER_MGR_IPC_LOCALTOUNIT:
            msg_p->type.ret_bool = XFER_MGR_LocalToUnit(
                &msg_p->data.arg_grp_bool_localtounit.arg_user_info,
                msg_p->data.arg_grp_bool_localtounit.arg_unit_id,
                msg_p->data.arg_grp_bool_localtounit.arg_destfile,
                msg_p->data.arg_grp_bool_localtounit.arg_srcfile,
                msg_p->data.arg_grp_bool_localtounit.arg_file_type,
                msg_p->data.arg_grp_bool_localtounit.arg_cookie,
                msg_p->data.arg_grp_bool_localtounit.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_localtounit.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_localtounit);
            break;

        case XFER_MGR_IPC_GETTFTPMGTENTRY:
        	msg_p->type.ret_bool = XFER_MGR_GetTftpMgtEntry(
        	    &msg_p->data.arg_grp_bool_gettftpmgtentry.arg_tftp_mgt_entry_Info);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_gettftpmgtentry);
            break;

        case XFER_MGR_IPC_SETTFTPFILETYPE:
        	msg_p->type.ret_ui32 = XFER_MGR_SetTftpFileType(
        	    msg_p->data.arg_grp_ui32_settftpfiletype.arg_tftp_file_type);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_ui32_settftpfiletype);
            break;

        case XFER_MGR_IPC_SETTFTPSRCFILE:
        	msg_p->type.ret_ui32 = XFER_MGR_SetTftpSrcFile(
        	    msg_p->data.arg_grp_ui32_settftpsrcfile.arg_tftp_src_file);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETTFTPDESTFILE:
        	msg_p->type.ret_ui32 = XFER_MGR_SetTftpDestFile(
        	    msg_p->data.arg_grp_ui32_settftpdestfile.arg_tftp_des_file);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETTFTPSERVER:
        	msg_p->type.ret_ui32 = XFER_MGR_SetTftpServer(
        	    &msg_p->data.arg_grp_ui32_settftpserver.arg_tftp_server);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETTFTPACTIVE:
            msg_p->type.ret_ui32 = XFER_MGR_SetTftpActive(
                &msg_p->data.arg_grp_ui32_settftpactive.arg_user_info,
                msg_p->data.arg_grp_ui32_settftpactive.arg_tftp_active);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_GETSWVERSION:
        	msg_p->type.ret_bool = XFER_MGR_GetSWVersion(
        	    msg_p->data.arg_grp_bool_getswversion.arg_file_name_p,
        	    msg_p->data.arg_grp_bool_getswversion.arg_software_version_p);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_getswversion);
            break;

        case XFER_MGR_IPC_GETTFTPERRORMSG:
        	msg_p->type.ret_bool = XFER_MGR_GetTftpErrorMsg(
        	    msg_p->data.arg_grp_bool_gettftperrormsg.arg_tftp_error_msg);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_gettftperrormsg);
            break;

        case XFER_MGR_IPC_GETFILECOPYMGTENTRY:
        	msg_p->type.ret_bool = XFER_MGR_GetFileCopyMgtEntry(
        	    &msg_p->data.arg_grp_bool_getfilecopymgtentry.arg_file_copy_mgt);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_getfilecopymgtentry);
            break;

        case XFER_MGR_IPC_SETFILECOPYSRCOPERTYPE:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopySrcOperType(
        	    msg_p->data.arg_grp_bool_setfilecopysrcopertype.arg_src_oper_type);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETFILECOPYSRCFILENAME:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopySrcFileName(
        	    msg_p->data.arg_grp_bool_setfilecopysrcfilename.arg_src_file_name_p);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETFILECOPYDESTOPERTYPE:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopyDestOperType(
        	    msg_p->data.arg_grp_bool_setfilecopydestopertype.arg_dest_oper_type);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETFILECOPYDESTFILENAME:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopyDestFileName(
        	    msg_p->data.arg_grp_bool_setfilecopydestfilename.arg_dest_file_name_p);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETFILECOPYFILETYPE:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopyFileType(
        	    msg_p->data.arg_grp_bool_setfilecopyfiletype.arg_file_type);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETFILECOPYTFTPSERVER:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopyTftpServer(
        	    &msg_p->data.arg_grp_bool_setfilecopytftpserver.arg_tftp_server);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETFILECOPYUNIT:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopyUnit(
        	    msg_p->data.arg_grp_bool_setfilecopyunit.arg_unit);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_SETFILECOPYACTION:
            msg_p->type.ret_bool = XFER_MGR_SetFileCopyAction(
                &msg_p->data.arg_grp_bool_streamtoremote.arg_user_info,
                msg_p->data.arg_grp_bool_setfilecopyaction.arg_action,
                msg_p->data.arg_grp_bool_setfilecopyaction.arg_cookie,
                msg_p->data.arg_grp_bool_setfilecopyaction.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_setfilecopyaction.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;
		case XFER_MGR_IPC_SETPUBLICKEYTYPE:
			msg_p->type.ret_bool = XFER_MGR_SetFileCopyPulicKeyType(
        	    msg_p->data.arg_grp_bool_setpublickeytype.arg_key_type);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
			break;
		case XFER_MGR_IPC_SETCOPYUSERNAME:
			msg_p->type.ret_bool = XFER_MGR_SetFileCopyUsername(
        	    msg_p->data.arg_grp_bool_setcopyusername.arg_username);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
			break;

        case XFER_MGR_IPC_ABORTTFTP:
        	msg_p->type.ret_bool = XFER_MGR_AbortTftp();
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (TRUE == SYS_CPNT_DBSYNC_TXT)
        case XFER_MGR_IPC_GETDUALSTARTUPCFGFILENAME:
        	msg_p->type.ret_bool = XFER_MGR_GetDualStartupCfgFileName(
        	    msg_p->data.arg_grp_bool_getdualstartupcfgfilename.arg_file_name);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_getdualstartupcfgfilename);
            break;
#endif /* #if (TRUE == SYS_CPNT_DBSYNC_TXT) */

        case XFER_MGR_IPC_AUTODOWNLOAD:
        	msg_p->type.ret_bool = XFER_MGR_AutoDownLoad(
        	    msg_p->data.arg_grp_bool_autodownload.arg_unit_list,
        	    msg_p->data.arg_grp_bool_autodownload.arg_destfile,
        	    msg_p->data.arg_grp_bool_autodownload.arg_srcfile,
        	    msg_p->data.arg_grp_bool_autodownload.arg_file_type,
        	    msg_p->data.arg_grp_bool_autodownload.arg_is_next_boot,
        	    msg_p->data.arg_grp_bool_autodownload.arg_cookie,
        	    msg_p->data.arg_grp_bool_autodownload.arg_ipc_message_q,
        	    msg_p->data.arg_grp_bool_autodownload.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_GETAUTODOWNLOAD_STATUS:
        	msg_p->type.ret_bool = XFER_MGR_GetAutoDownLoad_Status(
        	    msg_p->data.arg_grp_bool_getautodownload_status.arg_unit_id,
        	    &msg_p->data.arg_grp_bool_getautodownload_status.arg_download_status);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_getautodownload_status);
            break;

        case XFER_MGR_IPC_GETNEXTAUTODOWNLOAD_STATUS:
        	msg_p->type.ret_bool = XFER_MGR_GetNextAutoDownLoad_Status(
        	    &msg_p->data.arg_grp_bool_getnextautodownload_status.arg_unit_id,
        	    &msg_p->data.arg_grp_bool_getnextautodownload_status.arg_download_status);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_getnextautodownload_status);
            break;

        case XFER_MGR_IPC_AUTOCONFIGTOUNIT:
        	msg_p->type.ret_bool = XFER_MGR_AutoConfigToUnit(
        	    msg_p->data.arg_grp_bool_autoconfigtounit.arg_file_name,
        	    msg_p->data.arg_grp_bool_autoconfigtounit.arg_cookie,
        	    msg_p->data.arg_grp_bool_autoconfigtounit.arg_ipc_message_q,
        	    msg_p->data.arg_grp_bool_autoconfigtounit.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

#if defined(JBOS)
        case XFER_MGR_IPC_SETCHECKIMAGETYPE:
            XFER_MGR_SetCheckImageType(&msg_p->data.arg_grp_void_setcheckimagetype.arg_checkimagetype);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_void_setcheckimagetype);
            break;
#endif

        case XFER_MGR_IPC_SETSTARTUPFILENAME:
        	msg_p->type.ret_bool = XFER_MGR_SetStartupFilename(
        	    msg_p->data.arg_grp_bool_setstartupfilename.arg_file_type,
        	    msg_p->data.arg_grp_bool_setstartupfilename.arg_file_name,
        	    msg_p->data.arg_grp_bool_setstartupfilename.arg_cookie,
        	    msg_p->data.arg_grp_bool_setstartupfilename.arg_ipc_message_q,
        	    msg_p->data.arg_grp_bool_setstartupfilename.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_setstartupfilename);
            break;

        case XFER_MGR_IPC_WRITEFILE:
            buffer_pointer =(UI8_T *) BUFFER_MGR_GetPtr(msg_p->data.arg_grp_bool_writefile.arg_x_buf_offset);
            msg_p->type.ret_bool = XFER_MGR_WriteFile(
                &msg_p->data.arg_grp_bool_writefile.arg_user_info,
                msg_p->data.arg_grp_bool_writefile.arg_dest_file_name,
                msg_p->data.arg_grp_bool_writefile.arg_file_type,
                buffer_pointer,
                msg_p->data.arg_grp_bool_writefile.arg_length,
                msg_p->data.arg_grp_bool_writefile.arg_cookie,
                msg_p->data.arg_grp_bool_writefile.arg_ipc_message_q,
                msg_p->data.arg_grp_bool_writefile.arg_callback);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_GET_TFTP_RETRY_TIMES:
            msg_p->type.ret_bool = XFER_MGR_GetTftpRetryTimes(
                &msg_p->data.tftp_retry_times);
            break;

        case XFER_MGR_IPC_GET_RUNNING_TFTP_RETRY_TIMES:
            msg_p->type.ret_ui32 = XFER_MGR_GetRunningTftpRetryTimes(
                &msg_p->data.tftp_retry_times);
            break;

        case XFER_MGR_IPC_SET_TFTP_RETRY_TIMES:
            msg_p->type.ret_bool = XFER_MGR_SetTftpRetryTimes(
                msg_p->data.tftp_retry_times);
            break;

        case XFER_MGR_IPC_GET_TFTP_TIMEOUT:
            msg_p->type.ret_bool = XFER_MGR_GetTftpTimeout(
                &msg_p->data.tftp_timeout);
            break;

        case XFER_MGR_IPC_GET_RUNNING_TFTP_TIMEOUT:
            msg_p->type.ret_ui32 = XFER_MGR_GetRunningTftpTimeout(
                &msg_p->data.tftp_timeout);
            break;

        case XFER_MGR_IPC_SET_TFTP_TIMEOUT:
            msg_p->type.ret_bool = XFER_MGR_SetTftpTimeout(
                msg_p->data.tftp_timeout);
            break;

        case XFER_MGR_IPC_SETFILECOPYSERVERINETADDRESS:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopyServerInetAddress(
        	    &msg_p->data.arg_grp_bool_setserverinetaddress.arg_server_ip);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_setserverinetaddress);
            break;

        case XFER_MGR_IPC_SETFILECOPYSERVERUSERNAME:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopyServerUserName(
        	    msg_p->data.arg_grp_bool_setserverusername.arg_username);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_setserverusername);
            break;

        case XFER_MGR_IPC_SETFILECOPYSERVERPASSWORD:
        	msg_p->type.ret_bool = XFER_MGR_SetFileCopyServerPassword(
        	    msg_p->data.arg_grp_bool_setserverpassword.arg_password);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(arg_grp_bool_setserverpassword);
            break;

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
        case XFER_MGR_IPC_SETAUTOOPCODEUPGRADESTATUS:
        	msg_p->type.ret_bool = XFER_MGR_SetAutoOpCodeUpgradeStatus(
        	    msg_p->data.auto_upgrade_status);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_GETAUTOOPCODEUPGRADESTATUS:
        	msg_p->type.ret_bool = XFER_MGR_GetAutoOpCodeUpgradeStatus(
        	    &msg_p->data.auto_upgrade_status);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(auto_upgrade_status);
            break;

        case XFER_MGR_IPC_GETRUNNINGAUTOOPCODEUPGRADESTATUS:
        	msg_p->type.ret_ui32 = XFER_MGR_GetRunningAutoOpCodeUpgradeStatus(
        	    &msg_p->data.auto_upgrade_status);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(auto_upgrade_status);
            break;

        case XFER_MGR_IPC_SETAUTOOPCODEUPGRADERELOADSTATUS:
        	msg_p->type.ret_bool = XFER_MGR_SetAutoOpCodeUpgradeReloadStatus(
        	    msg_p->data.auto_upgrade_reload_status);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_GETAUTOOPCODEUPGRADERELOADSTATUS:
        	msg_p->type.ret_bool = XFER_MGR_GetAutoOpCodeUpgradeReloadStatus(
        	    &msg_p->data.auto_upgrade_reload_status);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(auto_upgrade_reload_status);
            break;

        case XFER_MGR_IPC_GETRUNNINGAUTOOPCODEUPGRADERELOADSTATUS:
        	msg_p->type.ret_ui32 = XFER_MGR_GetRunningAutoOpCodeUpgradeReloadStatus(
        	    &msg_p->data.auto_upgrade_reload_status);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(auto_upgrade_reload_status);
            break;

        case XFER_MGR_IPC_SETAUTOOPCODEUPGRADEPATH:
        	msg_p->type.ret_bool = XFER_MGR_SetAutoOpCodeUpgradePath(
        	    msg_p->data.auto_upgrade_path);
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XFER_MGR_IPC_GETAUTOOPCODEUPGRADEPATH:
        	msg_p->type.ret_bool = XFER_MGR_GetAutoOpCodeUpgradePath(
        	    msg_p->data.auto_upgrade_path);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(auto_upgrade_path);
            break;

        case XFER_MGR_IPC_GETRUNNINGAUTOOPCODEUPGRADEPATH:
        	msg_p->type.ret_ui32 = XFER_MGR_GetRunningAutoOpCodeUpgradePath(
        	    msg_p->data.auto_upgrade_path);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(auto_upgrade_path);
            break;

        case XFER_MGR_IPC_GETAUTOOPCODEUPGRADEFILENAME:
        	msg_p->type.ret_bool = XFER_MGR_GetAutoOpCodeUpgradeFileName(
        	    msg_p->data.auto_upgrade_filename);
            msgbuf_p->msg_size = XFER_MGR_GET_MSG_SIZE(auto_upgrade_filename);
            break;
#endif  /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
        case XFER_MGR_IPC_SET_PARTIAL_PROVISION_STATUS:
            XFER_MGR_SetPartialProvisionStatus(msg_p->data.partial_provision_status);
            return FALSE;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = XFER_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of XFER_MGR_HandleIPCReqMsg */

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
/* These code are workaround for the compiler warning:defined but not used
 * These can be removed when the compiler warning is not generated anymore.
 */
typedef void (*XFER_MGR_DummyFun_T) (void);
XFER_MGR_DummyFun_T XFER_MGR_DummyFunctionArrayForCompileWarnningWorkaround[]=
{

    (XFER_MGR_DummyFun_T)XFER_MGR_Check_File_Version_By_MText,
    (XFER_MGR_DummyFun_T)XFER_MGR_IsCheckImageVersion,
    (XFER_MGR_DummyFun_T)XFER_MGR_IsDebugFlagOn,
};
#endif

