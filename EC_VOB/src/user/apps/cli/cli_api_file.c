/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include "cli_api.h"
#include "cli_api_file.h"
#include "fs_type.h"
#include "fs.h"
#include "sys_time.h"
#include "buffer_mgr.h"
#include "l_inet.h"

#if (SYS_CPNT_SSH2 == TRUE)
#include "keygen_type.h"
#include "sshd_pmgr.h"
#include "userauth_pmgr.h"
#endif /* #if (SYS_CPNT_SSH2 == TRUE) */

#if (SYS_CPNT_HTTPS == TRUE)
#include "http_pmgr.h"
#endif /* #if (SYS_CPNT_HTTPS == TRUE) */

#include "ip_lib.h"
#include "netcfg_pmgr_route.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MSG_INVALID_FILE_NAME "Invalid file name.\r\n"
#define MSG_NO_SUCH_FILE "No such file.\r\n"
#define MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED "Factory Default Configuration file cannot be replaced.\r\n"
#define MSG_DEFAULT_CONFIG_CANNOT_BE_DELETED "Factory Default Configuration file cannot be deleted.\r\n"
#define MSG_STARTUP_CONFIG_CANNOT_BE_DELETED "Startup file cannot be deleted.\r\n"
#define MSG_COPY_ERROR "Copy error.\r\n"
#define MSG_UNIT_DOES_NOT_EXIST "Unit %lu does not exist.\r\n"
#define MSG_FAILED_TO_DELETE_FILE "Failed to delete file.\r\n"
#define MSG_SET_STARTUP_FILE_NAME_ERROR "Set startup file name error.\r\n"
#define MSG_INVALID_INPUT "Invalid input.\r\n"
#define MSG_FAILED_TO_ALLOCATE_RESOURCE "Failed to allocate resource.\r\n"
#define MSG_FAILED_TO_COPY_FROM_LOCAL_TO_RUNNING_CFG "Failed to copy from local to running configuration.\r\n"
#define MSG_FAILED_TO_COPY_FROM_REMOTE_TO_STREAM "Failed to copy from remote to stream\r\n"
#define MSG_FAILED_TO_SET_STARTUP_CONFIG "Failed to set startup configuration.\r\n"
#define MSG_FAILED_TO_COPY_RUNNING_CFG_TO_LOCAL "Failed to copy running configuration to local.\r\n"
#define MSG_FAILED_TO_COPY_RUNNING_CFG_FROM_REMOTE "Failed to copy running configuration from remote server.\r\n"
#define MSG_FAILED_TO_COPY_RUNNING_CFG_TO_REMOTE "Failed to copy running configuration to remote server.\r\n"
#define MSG_FAILED_TO_GET_STARTUP_CONFIG_FILE_NAME "Failed to get startup configuration file name.\r\n"
#define MSG_FAILED_TO_COPY_FILE "Failed to copy file.\r\n"
#define MSG_FAILED_TO_COPY_FROM_LOCAL_TO_REMOTE "Failed to copy from local to remote server.\r\n"
#define MSG_FAILED_TO_COPY_PUBLIC_KEY_FROM_REMOTE "Failed to copy public-key from remote server.\r\n"
#define MSG_FAILED_TO_GET_CERTIFICATE_FILE "Failed to get certificate file.\r\n"
#define MSG_FAILED_TO_GET_PRIVATE_FILE "Failed to get private file.\r\n"
#define MSG_FAILED_TO_WRITE_CERTIFICATE_FILE_TO_FLASH "Failed to write certificate file to flash.\r\n"
#define MSG_INVALID_USERNAME "Invalid username.\r\n"
#define MSG_NO_SUCH_USER "No such user.\r\n"
#define MSG_NO_INVALID_USERNAME "Invalid username.\r\n"

#define MSG_COPY_FILE_TYPE_SRC_CONFIG "Source configuration file name: "
#define MSG_COPY_FILE_TYPE_DST_CONFIG "Destination configuration file name: "
#define MSG_COPY_FILE_TYPE_SRC_GENERAL "Source file name: "
#define MSG_COPY_FILE_TYPE_DST_GENERAL "Destination file name: "
#define MSG_COPY_FILE_TYPE_CERTIFICATE "Source certificate file name: "
#define MSG_COPY_FILE_TYPE_PRIVATE_KEY "Source private-key file name: "
#define MSG_COPY_FILE_TYPE_PUBLIC_KEY "Source public-key file name: "

#define USB_PATH_LENGTH (XFER_TYPE_MAX_SIZE_OF_REMOTE_FILE_NAME)
#define USB_FILEOPER_MGR_BLOCKSIZE (512 * 2 * 32)
#define USB_MAXSIZE_DestFile (XFER_TYPE_MAX_SIZE_OF_REMOTE_FILE_NAME)

/* MACRO FUNCTION DECLARATIONS
 */
#define COPY_FILE_TYPE_CONFIG {FS_FILE_TYPE_CONFIG, "config"}
#define COPY_FILE_TYPE_RUNTIME {FS_TYPE_OPCODE_FILE_TYPE, "opcode"}
#define COPY_FILE_TYPE_KERNEL {FS_FILE_TYPE_KERNEL, "kernel"}
#define COPY_FILE_TYPE_TOTAL {FS_FILE_TYPE_TOTAL, "loader"}
#define COPY_FILE_TYPE_DIAG {FS_FILE_TYPE_DIAG, "diag"}
#define COPY_FILE_TYPE_LICENSE {FS_FILE_TYPE_LICENSE, "license"}

/* DATA TYPE DECLARATIONS
 */
typedef void (*CLI_API_FILE_CopyFuncPtr_T)(XFER_MGR_RemoteServer_T server_type);

typedef enum COPY_E
{
    COPY_FILE = 0,
    COPY_REMOTE,
    COPY_STARTUP_CFG,
    COPY_RUNNING_CFG,
    COPY_CERTIFICATE,
    COPY_PUBLIC_KEY,
    COPY_UNIT,
    COPY_USBDISK,
    COPY_TO_CURRENT_RUN,    /* add-to-running-config */

    COPY_MAX_NUM,
} COPY_T;

typedef struct
{
    L_INET_AddrIp_T ip_address;

    char login_username[MAXSIZE_fileCopyServerUserName + 1];
    char login_password[MAXSIZE_fileCopyServerPassword + 1];
} COPY_FILE_ServerInfo_T;

typedef struct COPY_FILE_TYPE_S
{
    FS_File_Type_T type;
    char name[10 + 1];
} CLI_API_FILE_TYPE_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T show_file_info(UI32_T unit, UI32_T filetype, char *filename, BOOL_T is_startup_only, UI32_T line_num);
static BOOL_T waiting_xfer_result(void);
static BOOL_T is_file_existed(UI32_T unit, UI32_T file_type, char *filename, BOOL_T *is_startup);
static BOOL_T is_current_unit(UI32_T unit_id);
static BOOL_T is_default_config_file(char *filename);
static BOOL_T sync_config_file(char *filename);
static BOOL_T parse_copy_type(char *arg, COPY_T *copy_type, XFER_MGR_RemoteServer_T *remote_type);
static BOOL_T input_decision();
static BOOL_T input_remote_server_info(XFER_MGR_RemoteServer_T server_type, COPY_FILE_ServerInfo_T *server_info_p);
static BOOL_T input_file_type(CLI_API_FILE_TYPE_T *available_list, const UI32_T available_list_len, FS_File_Type_T *input_value);

#if (SYS_CPNT_SSH2 == TRUE)
static BOOL_T input_public_key_type(UI32_T *key_type_p);
static BOOL_T input_username(char *username, UI32_T username_len);
#endif  /* #if (SYS_CPNT_SSH2 == TRUE) */

#if (SYS_CPNT_HTTP_UI == TRUE)
static BOOL_T input_password(char *password, UI32_T password_len);
#endif /* #if (SYS_CPNT_HTTP_UI == TRUE) */

static void notify_to_enter_transition_mode(UI8_T *xfer_buf);
static BOOL_T confirm_to_reset();
static BOOL_T input_filename(char *message, char *filename, UI32_T filename_len);
static BOOL_T input_startup_file_name(char *filename, UI32_T filename_len, BOOL_T *is_startup_cfg_need_to_change);
static BOOL_T mount_usbdisk();
static BOOL_T umount_usbdisk();
static void dir_usbdisk(UI8_T *path);
static void dir_usbdisk_filename(char *filename);

static void copy_from_file_to_file(XFER_MGR_RemoteServer_T server_type);
static void copy_from_file_to_remote(XFER_MGR_RemoteServer_T server_type);
static void copy_from_file_to_startup_cfg(XFER_MGR_RemoteServer_T server_type);
static void copy_from_file_to_running_cfg(XFER_MGR_RemoteServer_T server_type);
static void copy_from_file_to_unit(XFER_MGR_RemoteServer_T server_type);
static void copy_from_remote_to_file(XFER_MGR_RemoteServer_T server_type);
static void copy_from_remote_to_certificate(XFER_MGR_RemoteServer_T server_type);
static void copy_from_remote_to_public_key(XFER_MGR_RemoteServer_T server_type);
static void copy_from_remote_to_running_cfg(XFER_MGR_RemoteServer_T server_type);
static void copy_from_remote_to_add_run(XFER_MGR_RemoteServer_T server_type);
static void copy_from_remote_to_startup_cfg(XFER_MGR_RemoteServer_T server_type);
static void copy_from_running_cfg_to_file(XFER_MGR_RemoteServer_T server_type);
static void copy_from_running_cfg_to_remote(XFER_MGR_RemoteServer_T server_type);
static void copy_from_running_cfg_to_startup_cfg(XFER_MGR_RemoteServer_T server_type);
static void copy_from_startup_cfg_to_file(XFER_MGR_RemoteServer_T server_type);
static void copy_from_startup_cfg_to_remote(XFER_MGR_RemoteServer_T server_type);
static void copy_from_startup_cfg_to_running_cfg(XFER_MGR_RemoteServer_T server_type);
static void copy_from_unit_to_file(XFER_MGR_RemoteServer_T server_type);
static void copy_from_usbdisk_to_file(XFER_MGR_RemoteServer_T server_type);
static void copy_from_file_To_usbdisk(XFER_MGR_RemoteServer_T server_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - get_file_copy_user_info
 *------------------------------------------------------------------------
 * FUNCTION: Get file copy user information
 * INPUT   : user_info_p  -- user information entry
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
get_file_copy_user_info(
    XFER_MGR_UserInfo_T *user_info_p
);

/* STATIC VARIABLE DEFINITIONS
 */
static CLI_API_FILE_CopyFuncPtr_T copy_func_array[COPY_MAX_NUM][COPY_MAX_NUM] =
{
    /*                         COPY_FILE                      COPY_REMOTE                      COPY_STARTUP_CFG                      COPY_RUNNING_CFG                      COPY_CERTIFICATE                 COPY_PUBLIC_KEY                 COPY_UNIT               COPY_USBDISK               COPY_TO_CURRENT_RUN*/
    /* COPY_FILE        */    {copy_from_file_to_file,        copy_from_file_to_remote,        copy_from_file_to_startup_cfg,        copy_from_file_to_running_cfg,        NULL,                            NULL,                           copy_from_file_to_unit, copy_from_file_To_usbdisk, NULL},
    /* COPY_REMOTE      */    {copy_from_remote_to_file,      NULL,                            copy_from_remote_to_startup_cfg,      copy_from_remote_to_running_cfg,      copy_from_remote_to_certificate, copy_from_remote_to_public_key, NULL,                   NULL,                      copy_from_remote_to_add_run},
    /* COPY_STARTUP_CFG */    {copy_from_startup_cfg_to_file, copy_from_startup_cfg_to_remote, NULL,                                 copy_from_startup_cfg_to_running_cfg, NULL,                            NULL,                           NULL,                   NULL,                      NULL},
    /* COPY_RUNNING_CFG */    {copy_from_running_cfg_to_file, copy_from_running_cfg_to_remote, copy_from_running_cfg_to_startup_cfg, NULL,                                 NULL,                            NULL,                           NULL,                   NULL,                      NULL},
    /* COPY_CERTIFICATE */    {NULL,                          NULL,                            NULL,                                 NULL,                                 NULL,                            NULL,                           NULL,                   NULL,                      NULL},
    /* COPY_PUBLIC_KEY  */    {NULL,                          NULL,                            NULL,                                 NULL,                                 NULL,                            NULL,                           NULL,                   NULL,                      NULL},
    /* COPY_UNIT        */    {copy_from_unit_to_file,        NULL,                            NULL,                                 NULL,                                 NULL,                            NULL,                           NULL,                   NULL,                      NULL},
    /* COPY_USBDISK     */    {copy_from_usbdisk_to_file,     NULL,                            NULL,                                 NULL,                                 NULL,                            NULL,                           NULL,                   NULL,                      NULL},
    /* COPY_TO_CURRENT_RUN */ {NULL,                          NULL,                            NULL,                                 NULL,                                 NULL,                            NULL,                           NULL,                   NULL,                      NULL},
};

/* EXPORTED SUBPROGRAM BODIES
 */
UI32_T CLI_API_Copy(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    CLI_API_FILE_CopyFuncPtr_T copy;
    COPY_T from_type;
    COPY_T to_type;
    XFER_MGR_RemoteServer_T from_server_type;
    XFER_MGR_RemoteServer_T to_server_type;
    XFER_MGR_RemoteServer_T server_type;

    if (   (FALSE == parse_copy_type(arg[0], &from_type, &from_server_type))
        || (FALSE == parse_copy_type(arg[1], &to_type, &to_server_type)))
    {
        return CLI_ERR_INTERNAL;
    }

    server_type = (XFER_MGR_REMOTE_SERVER_NONE != from_server_type) ? from_server_type : to_server_type;

    copy = copy_func_array[from_type][to_type];

    if (NULL == copy)
    {
        char buff[CLI_DEF_MAX_BUFSIZE + 1] = {0};

        sprintf(buff, "Unhandled copy type (from_type = %d, to_type = %d, server_type = %d).\r\n", from_type, to_type, server_type);
        CLI_LIB_PrintStr(buff);

        return CLI_ERR_INTERNAL;
    }

    copy(server_type);
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Write(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    XFER_MGR_UserInfo_T  user_info;
    COPY_FILE_ServerInfo_T server_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};

    memset(&server_info, 0, sizeof(server_info));

    if ((FS_RETURN_OK != FS_GetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, (UI8_T *)dst_filename))
        || ('\0' == dst_filename[0]))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_GET_STARTUP_CONFIG_FILE_NAME);
        return CLI_ERR_INTERNAL;
    }

    if (TRUE == is_default_config_file(dst_filename))
    {
        CLI_LIB_PrintStr_1("Cannot overwrite read-only %s configuration file.\r\n", dst_filename);
        return CLI_NO_ERROR;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        XFER_MGR_STREAM_TO_STARTUP,
        XFER_MGR_REMOTE_SERVER_NONE,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_RUNNING_CFG_TO_LOCAL);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return CLI_ERR_INTERNAL;
    }

    if (FALSE == waiting_xfer_result())
    {
        return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Delete_File(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    FS_File_Attr_T file_attr;
    UI32_T unit_id;
    UI8_T  master_unit_id;

    if (NULL == arg[0] ||
        NULL == arg[1])
    {
        return CLI_ERR_INTERNAL;
    }

    memset(&file_attr, 0, sizeof(FS_File_Attr_T));

    switch (arg[0][0])
    {
        /* name
         */
        case 'n':
        case 'N':
            unit_id = DUMMY_DRIVE;
            strncpy((char *)file_attr.file_name, arg[1], sizeof(file_attr.file_name)-1);
            file_attr.file_name[sizeof(file_attr.file_name)-1] = '\0';
            break;

        /* unit
         */
        case 'u':
        case 'U':
            unit_id = atoi(arg[1]);

            if (TRUE != STKTPLG_POM_GetMasterUnitId(&master_unit_id))
            {
                return CLI_ERR_INTERNAL;
            }

            if (unit_id == master_unit_id)
            {
                unit_id = DUMMY_DRIVE;
            }
            else
            {
                if (TRUE != STKTPLG_POM_UnitExist(unit_id))
                {
                    CLI_LIB_PrintStr_1(MSG_UNIT_DOES_NOT_EXIST, (unsigned long)unit_id);
                    return CLI_NO_ERROR;
                }
            }

            if (NULL == arg[2] ||
                NULL == arg[3])
            {
                return CLI_ERR_INTERNAL;
            }

            switch (arg[2][0])
            {
                /* name
                 */
                case 'n':
                case 'N':
                    strncpy((char *)file_attr.file_name, arg[3], sizeof(file_attr.file_name)-1);
                    file_attr.file_name[sizeof(file_attr.file_name)-1] = '\0';
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_FILE_TYPE_DIAG)    |
                               FS_FILE_TYPE_MASK(FS_TYPE_OPCODE_FILE_TYPE) |
                               FS_FILE_TYPE_MASK(FS_FILE_TYPE_KERNEL)  |
                               FS_FILE_TYPE_MASK(FS_FILE_TYPE_CONFIG);

    if (FS_RETURN_OK != FS_GetFileInfo(unit_id, &file_attr))
    {
        CLI_LIB_PrintStr(MSG_NO_SUCH_FILE);
    }
    else if (file_attr.file_type == FS_FILE_TYPE_CONFIG &&
        strcmp((char *)file_attr.file_name, SYS_DFLT_restartConfigFile) == 0)
    {
        CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_DELETED);
    }
    else if (TRUE == file_attr.startup_file)
    {
        CLI_LIB_PrintStr(MSG_STARTUP_CONFIG_CANNOT_BE_DELETED);
    }
    else if (FS_RETURN_OK != FS_DeleteFile(unit_id, file_attr.file_name))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_DELETE_FILE);
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dir(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T unit_id = DUMMY_DRIVE;
    UI32_T total_size_of_free_space;
    UI32_T line_num = 0;
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};

/*gxz add,for mpc8347 usb,2009-04-09,begin*/
    if(NULL != arg[0])
    {
        if(*arg[0] == 'u' ||*arg[0] == 'U')
        {
            dir_usbdisk((UI8_T *)arg[1]);
            return CLI_NO_ERROR;
        }
    }
/*gxz add,for mpc8347 usb,2009-04-09,end*/

    sprintf(buff,  "File Name                      Type    Startup Modified Time       Size (bytes)\r\n");
    PROCESS_MORE(buff);
    sprintf(buff,  "------------------------------ ------- ------- ------------------- ------------\r\n");
    PROCESS_MORE(buff);

    if(arg[0] == NULL) /*all file type*//*all files*/
    {
        UI32_T i, j, file_type[] = {FS_FILE_TYPE_DIAG, FS_FILE_TYPE_KERNEL, FS_TYPE_OPCODE_FILE_TYPE, FS_FILE_TYPE_CONFIG};
        UI32_T num_of_unit;
#if (SYS_CPNT_STACKING == TRUE)
        //STKTPLG_MGR_GetNumberOfUnit(&num_of_unit);
        num_of_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
        num_of_unit = 1;
#endif
        /*   for (j = 1; j <= num_of_unit; j++) */
        for (j=0; STKTPLG_POM_GetNextUnit(&j);)
        {
#if (SYS_CPNT_STACKING == TRUE)
            sprintf(buff, " Unit %lu:\r\n",(unsigned long)j);
            PROCESS_MORE(buff);
#endif
            for(i = 0; i<sizeof(file_type)/sizeof(UI32_T); i++)
            {

                line_num = show_file_info(j, file_type[i], 0, FALSE, line_num);/*pttch stacking*/
                if (line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
            }

            if (FS_GetStorageFreeSpace(j, &total_size_of_free_space) == FS_RETURN_OK)
            {
                sprintf(buff,"-------------------------------------------------------------------------------\r\n");
                PROCESS_MORE(buff);
                sprintf(buff,"                        Free space for compressed user config files: %10lu\r\n",(unsigned long)total_size_of_free_space);
                PROCESS_MORE(buff);
            }
        }
    }
    else
    {
        switch(*arg[0])
        {
            case 'b':
            case 'B':
                line_num = show_file_info(unit_id, FS_FILE_TYPE_DIAG, arg[1], FALSE, line_num);
                if (line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                break;

            case 'c':
            case 'C':
                line_num = show_file_info(unit_id, FS_FILE_TYPE_CONFIG, arg[1], FALSE, line_num);
                if (line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                break;

            case 'o':
            case 'O':
                line_num = show_file_info(unit_id, FS_TYPE_OPCODE_FILE_TYPE, arg[1], FALSE, line_num);
                if (line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                break;

            default:/*pttch stacking*/
#if (SYS_CPNT_STACKING == TRUE)
                unit_id = atoi(arg[0]);

                if (!STKTPLG_POM_UnitExist(unit_id))
                {

#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    sprintf(buff, MSG_UNIT_DOES_NOT_EXIST, (unsigned long)unit_id);
                    PROCESS_MORE(buff);
#endif
                    return CLI_NO_ERROR;
                }

                if(arg[1]!=NULL)
                {
                    switch(*arg[1])
                    {
                        case 'b':
                        case 'B': /*pttch stacking*/
                            line_num = show_file_info(unit_id, FS_FILE_TYPE_DIAG, arg[2], FALSE, line_num);
                            break;

                        case 'c':
                        case 'C':/*pttch stacking*/
                            line_num = show_file_info(unit_id, FS_FILE_TYPE_CONFIG, arg[2], FALSE, line_num);
                            break;

                        case 'o':
                        case 'O':/*pttch stacking*/
                        default:
                            line_num = show_file_info(unit_id, FS_TYPE_OPCODE_FILE_TYPE, arg[2], FALSE, line_num);
                            break;
                    }

                    if (line_num == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }
                }
                else
                {  /*show specify unit all file*/
                    UI32_T i = 0;
                    UI32_T  file_type[] = {FS_FILE_TYPE_DIAG, FS_TYPE_OPCODE_FILE_TYPE, FS_FILE_TYPE_CONFIG};

                    sprintf(buff," Unit %lu:\r\n",(unsigned long)unit_id);
                    PROCESS_MORE(buff);

                    for(i = 0; i<sizeof(file_type)/sizeof(UI32_T); i++)
                    {
                        line_num = show_file_info(unit_id, file_type[i], 0, FALSE, line_num);/*pttch stacking*/
                        if (line_num == JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                    }
                }
#endif
                break;/*arg[0] default's break*/
        }


        if (FS_GetStorageFreeSpace(unit_id, &total_size_of_free_space) == FS_RETURN_OK)
        {
            sprintf(buff,"-------------------------------------------------------------------------------\r\n");
            PROCESS_MORE(buff);
            sprintf(buff,"                        Free space for compressed user config files: %10lu\r\n",(unsigned long)total_size_of_free_space);
            PROCESS_MORE(buff);
        }
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Boot_System(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T file_type = FS_FILE_TYPE_TOTAL;
    UI32_T unit_id = 0;
    char filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};

    /* argument format:
     *  [unit:]{boot-rom: | config: | opcode: } file-name
     *
     * arg[0] is necessary
     */
    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    if (isdigit(arg[0][0]))
    {
        UI8_T master_unit_id=0;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        unit_id = atoi(arg[0]);

        if (unit_id == master_unit_id)
        {
            unit_id = DUMMY_DRIVE;
        }
        else
        {
            if (!STKTPLG_POM_UnitExist(unit_id))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1(MSG_UNIT_DOES_NOT_EXIST, (unsigned long)unit_id);
#endif
                return CLI_NO_ERROR;
            }
        }

        if((arg[1]==NULL) || (arg[2]==NULL))
            return CLI_ERR_INTERNAL;

        strcpy(filename, arg[2]);

        switch(*arg[1])
        {
            /*boot-rom*/
            case 'b':
            case 'B':
                file_type = FS_FILE_TYPE_DIAG;
                break;

            /*config*/
            case 'c':
            case 'C':
                file_type = FS_FILE_TYPE_CONFIG;
                break;

            /*opcode*/
            case 'o':
            case 'O':
                file_type = FS_TYPE_OPCODE_FILE_TYPE;
                break;
            /* linux kernel */
            case 'k':
            case 'K':
                file_type = FS_FILE_TYPE_KERNEL;
                break;
            default:
                return CLI_ERR_INTERNAL;
        }
    }
    else
    {
        unit_id = DUMMY_DRIVE;

        if(arg[1]==NULL)
            return CLI_ERR_INTERNAL;

        strcpy(filename, arg[1]);
        switch(*arg[0])
        {
            /*boot-rom*/
            case 'b':
            case 'B':
                file_type = FS_FILE_TYPE_DIAG;
                break;

            /*config*/
            case 'c':
            case 'C':
                file_type = FS_FILE_TYPE_CONFIG;
                break;
            /*opcode*/
            case 'o':
            case 'O':
                file_type = FS_TYPE_OPCODE_FILE_TYPE;
                break;
            /* linux kernel */
            case 'k':
            case 'K':
                file_type = FS_FILE_TYPE_KERNEL;
                break;
            default:
                return CLI_ERR_INTERNAL;
        }
    }

    if(file_type != FS_FILE_TYPE_TOTAL)
    {
        BOOL_T is_startup;

        if(!is_file_existed(unit_id, file_type, filename, &is_startup))
        {
           CLI_LIB_PrintStr(MSG_NO_SUCH_FILE);
           return CLI_NO_ERROR;
        }
        if(FS_SetStartupFilename(unit_id, file_type, (UI8_T *)filename)!=FS_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr(MSG_SET_STARTUP_FILE_NAME_ERROR);
#endif
            return CLI_NO_ERROR;
        }
    }
   if (   file_type == FS_TYPE_OPCODE_FILE_TYPE
       || file_type == FS_FILE_TYPE_CONFIG
       || file_type == FS_FILE_TYPE_KERNEL
#if (SYS_CPNT_XFER_DOWNLOAD_DIAG == TRUE)
       || file_type == FS_FILE_TYPE_DIAG
#endif
      )
    {
        UI8_T my_unit_id = 0, i;
        UI8_T   unit_list[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK] = {0};
        char   buff1[15] = {0};
        char   buff[15] = {0};
        BOOL_T  stacking = FALSE;

        if (unit_id == DUMMY_DRIVE)
        {
            STKTPLG_POM_GetMasterUnitId(&my_unit_id);
            /*check current dut in stacking or stand alone mode if stacking syn to slave, if not ,return true*/
            for(i=1; i<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
            {
                if(i == my_unit_id)
                    continue;

                if(STKTPLG_POM_UnitExist(i))
                {
                    stacking = TRUE;
                    break;
                }
            }

            CLI_MGR_Set_XferProgressStatus(TRUE);
            CLI_MGR_Set_XferProgressResult(FALSE);

            if (FALSE == XFER_PMGR_AutoDownLoad(unit_list,
                (UI8_T *)filename,
                (UI8_T *)filename,
                file_type,
                TRUE,
                (void *)ctrl_P,
                SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
                NULL))
            {
                CLI_LIB_PrintStr(MSG_COPY_ERROR);
                CLI_MGR_Set_XferProgressStatus(FALSE);
                CLI_MGR_Set_XferProgressResult(FALSE);
                return CLI_NO_ERROR;
            }

            waiting_xfer_result();

            for(i=1; i<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
            {
                if(1==unit_list[i-1])
                {
                    sprintf(buff1, "%d, ", i);
                }

                strcat(buff, buff1);
            }
        }
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_AutoImage_Download(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Whichboot(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T i, j, file_type[] = {FS_FILE_TYPE_DIAG, FS_TYPE_OPCODE_FILE_TYPE, FS_FILE_TYPE_CONFIG};
    UI32_T unit_id,num_of_unit;
    UI32_T line_num = 0;
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    sprintf(buff,  "File Name                      Type    Startup Modified Time       Size (bytes)\r\n");
    PROCESS_MORE(buff);
    sprintf(buff,  "------------------------------ ------- ------- ------------------- ------------\r\n");
    PROCESS_MORE(buff);

#if (SYS_CPNT_STACKING == TRUE)
    //STKTPLG_MGR_GetNumberOfUnit(&num_of_unit);
    num_of_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    num_of_unit = 1;
#endif

    if (arg[0] == NULL)
    {
        for (j = 0; STKTPLG_POM_GetNextUnit(&j); )
        {
#if (SYS_CPNT_STACKING == TRUE)
            sprintf(buff," Unit %lu:\r\n",(unsigned long)j);
            PROCESS_MORE(buff);
#endif
            for(i = 0; i<sizeof(file_type)/sizeof(UI32_T); i++)/*pttch stacking*/
            {
                line_num = show_file_info(j, file_type[i], 0, TRUE, line_num);
                if (line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
            }
        }
    }
    else
    {
        unit_id = atoi(arg[0]);

        if (!STKTPLG_POM_UnitExist(unit_id))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            sprintf(buff, MSG_UNIT_DOES_NOT_EXIST, (unsigned long)unit_id);
            PROCESS_MORE(buff);
#endif
            return CLI_NO_ERROR;
        }
        sprintf(buff," Unit %lu:\r\n",(unsigned long)unit_id);
        PROCESS_MORE(buff);
        for(i = 0; i<sizeof(file_type)/sizeof(UI32_T); i++)/*pttch stacking*/
        {
            line_num = show_file_info(unit_id, file_type[i], 0, TRUE, line_num);
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }

    return CLI_NO_ERROR;
}

/*gxz add,for mpc8347 usb,2009-04-09,begin*/
UI32_T CLI_API_Delete_File_Usbdisk(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T input_str[USB_PATH_LENGTH];
    int i;

    if(TRUE != mount_usbdisk())
    {
        CLI_LIB_PrintStr("Usbdisk is not ready.\r\n");
        return CLI_NO_ERROR;
    }

    memset(input_str,0,sizeof(input_str));
    strcpy((char*)input_str,"rm -rf /mnt/usb/");

    /*arg[0] is the target file/directory name*/
    if(NULL == arg[1])
    {
        return CLI_NO_ERROR;
    }

    /*skip '/' from the beginning of arg[0]*/
    i =0;
    while('/' == arg[1][i]) i++;
    strcat((char*)input_str,&arg[1][i]);

    system((char*)input_str);

    return CLI_NO_ERROR;
}


UI32_T CLI_API_Umount_Usbdisk(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    if(TRUE != umount_usbdisk())
    {
        CLI_LIB_PrintStr("Failed to umount usbdisk.\r\n");
    }
    else
    {
        CLI_LIB_PrintStr("You can safely remove your usbdisk.\r\n");
    }

    return CLI_NO_ERROR;
}
/*gxz add,for mpc8347 usb,2009-04-09,end*/

/*command: [no] upgrade opcode auto*/
UI32_T CLI_API_Upgrade_Opcode_Auto(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    BOOL_T ret;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_UPGRADE_OPCODE_AUTO:
            ret = XFER_PMGR_SetAutoOpCodeUpgradeStatus(VAL_fileAutoUpgradeOpCodeStatus_enabled);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_UPGRADE_OPCODE_AUTO:
            ret = XFER_PMGR_SetAutoOpCodeUpgradeStatus(VAL_fileAutoUpgradeOpCodeStatus_disabled);
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (FALSE == ret)
    {
        CLI_LIB_PrintStr("Failed to set auto opcode upgrade\r\n");
    }
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */
    return CLI_NO_ERROR;
}

/*command: [no] upgrade opcode reload*/
UI32_T CLI_API_Upgrade_Opcode_Reload(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    BOOL_T ret;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_UPGRADE_OPCODE_RELOAD:
            ret = XFER_PMGR_SetAutoOpCodeUpgradeReloadStatus(VAL_fileAutoUpgradeOpCodeReloadStatus_enabled);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_UPGRADE_OPCODE_RELOAD:
            ret = XFER_PMGR_SetAutoOpCodeUpgradeReloadStatus(VAL_fileAutoUpgradeOpCodeReloadStatus_disabled);
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (FALSE == ret)
    {
        CLI_LIB_PrintStr("Failed to set auto opcode upgrade reload\r\n");
    }
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */
    return CLI_NO_ERROR;
}

/*command:    upgrade opcode path opcode-dir-url
           no upgrade opcode path
 */
UI32_T CLI_API_Upgrade_Opcode_Path(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    BOOL_T ret;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_UPGRADE_OPCODE_PATH:
            ret = XFER_PMGR_SetAutoOpCodeUpgradePath(arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_UPGRADE_OPCODE_PATH:
            ret = XFER_PMGR_SetAutoOpCodeUpgradePath(SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_PATH);
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (FALSE == ret)
    {
        CLI_LIB_PrintStr("Failed to set opcode path\r\n");
    }
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */

    return CLI_NO_ERROR;
}

/*command: show upgrade*/
UI32_T CLI_API_Show_Upgrade(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    UI32_T status;
    UI32_T reload_status;
    char  path[MAXSIZE_fileAutoUpgradeOpCodePath + 1] = {0};
    char  file[MAXSIZE_fileAutoUpgradeOpCodeFileName + 1] = {0};

    XFER_PMGR_GetAutoOpCodeUpgradeStatus(&status);
    XFER_PMGR_GetAutoOpCodeUpgradeReloadStatus(&reload_status);
    XFER_PMGR_GetAutoOpCodeUpgradePath(path);
    XFER_PMGR_GetAutoOpCodeUpgradeFileName(file);

    CLI_LIB_PrintStr  ("Auto Image Upgrade Global Settings:\r\n");
    CLI_LIB_PrintStr_1("  Status    : %s\r\n", (status == VAL_fileAutoUpgradeOpCodeStatus_enabled) ? "Enabled" : "Disabled");
    CLI_LIB_PrintStr_1("  Reload Status : %s\r\n", (reload_status == VAL_fileAutoUpgradeOpCodeReloadStatus_enabled) ? "Enabled" : "Disabled");
    CLI_LIB_PrintStr_1("  Path      : %s\r\n", path);
    CLI_LIB_PrintStr_1("  File Name : %s\r\n", file);
#endif /*#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)*/

    return CLI_NO_ERROR;
}

UI32_T CLI_API_IP_TFTP_Retry(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T retry_times = SYS_DFLT_TFTP_NUMBER_OF_RETRIES;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_TFTP_RETRY:
            retry_times = (UI32_T)atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_TFTP_RETRY:
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (FALSE == XFER_PMGR_SetTftpRetryTimes(retry_times))
    {
        CLI_LIB_PrintStr("Failed to set TFTP retry times\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_IP_TFTP_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T timeout = XFER_TYPE_DFLT_TFTP_TIMEOUT;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_TFTP_TIMEOUT:
            timeout = (UI32_T)atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_TFTP_TIMEOUT:
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (FALSE == XFER_PMGR_SetTftpTimeout(timeout))
    {
        CLI_LIB_PrintStr("Failed to set TFTP timeout\r\n");
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_IP_TFTP(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  retry_times = 0;
    UI32_T  timeout     = 0;

    CLI_LIB_PrintStr("TFTP Settings:\r\n");

    XFER_PMGR_GetTftpRetryTimes(&retry_times);
    CLI_LIB_PrintStr_1("  Retries : %lu\r\n", (unsigned long)retry_times);

    XFER_PMGR_GetTftpTimeout(&timeout);
    CLI_LIB_PrintStr_1("  Timeout : %lu seconds\r\n", (unsigned long)timeout);

    return CLI_NO_ERROR;
}

/* LOCAL SUBPROGRAM BODIES
 */
#define FILENAME_LEN_IN_ONE_LINE 30
static UI32_T show_file_info(UI32_T unit, UI32_T filetype, char *filename, BOOL_T is_startup_only, UI32_T line_num)
{
    FS_File_Attr_T file_attr;
    char   tmp_buf[160] = {0};
    UI32_T show_unit = unit;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    int    year, month, day, hour, minute, second;
    UI32_T filename_len;
    UI8_T  filename_buff[FILENAME_LEN_IN_ONE_LINE + 1] = "";
    BOOL_T end_of_1st_line = FALSE;
    UI32_T index = 0;

    if(filename == 0) /*all files*/
    {
        memset(&file_attr, 0, sizeof(FS_File_Attr_T));
        file_attr.file_type_mask = FS_FILE_TYPE_MASK(filetype);

        while(FS_GetNextFileInfo(&show_unit,&file_attr)==FS_RETURN_OK)
        {
            if (show_unit != unit)
                break;
            if(is_startup_only && file_attr.startup_file == FALSE)
                continue;

            SYS_TIME_ConvertSecondsToDateTime(file_attr.create_time, &year, &month, &day, &hour, &minute, &second);

            if (strlen((char*)file_attr.file_name) <= FILENAME_LEN_IN_ONE_LINE)
            {
                sprintf(tmp_buf, "%-30s %-7s %-4c    %04d-%02d-%02d %02d:%02d:%02d  %11lu\r\n", file_attr.file_name,
                    file_attr.file_type == FS_FILE_TYPE_KERNEL ? "Kernel Image": (file_attr.file_type == FS_TYPE_OPCODE_FILE_TYPE ? "OpCode" : "Config"),
                    file_attr.startup_file == TRUE? 'Y' : 'N',
                    year, month, day, hour, minute, second,
                    (unsigned long)file_attr.file_size);
                PROCESS_MORE_FUNC(tmp_buf);
            }
            else
            {
                end_of_1st_line = FALSE;
                index = 0;
                filename_len = strlen((char*)file_attr.file_name);

                while (filename_len > 0)
                {
                    if (filename_len >= FILENAME_LEN_IN_ONE_LINE)
                    {
                        memcpy(filename_buff, file_attr.file_name + index, FILENAME_LEN_IN_ONE_LINE);
                        filename_len -= FILENAME_LEN_IN_ONE_LINE;
                        index += FILENAME_LEN_IN_ONE_LINE;
                    }
                    else
                    {
                        memcpy(filename_buff, file_attr.file_name + index, filename_len);
                        filename_buff[filename_len] = '\0';
                        filename_len -= filename_len;
                    }
                    if (end_of_1st_line != TRUE)
                    {
                        sprintf(tmp_buf, "%-30s %-7s %-4c    %04d-%02d-%02d %02d:%02d:%02d  %11lu\r\n", filename_buff,
                            file_attr.file_type == FS_FILE_TYPE_KERNEL ? "Kernel Image" : (file_attr.file_type == FS_TYPE_OPCODE_FILE_TYPE ? "OpCode" : "Config"),
                            file_attr.startup_file == TRUE? 'Y' : 'N',
                            year, month, day, hour, minute, second,
                            (unsigned long)file_attr.file_size);

                        PROCESS_MORE_FUNC(tmp_buf);
                        end_of_1st_line = TRUE;
                    }
                    else
                    {
                        sprintf(tmp_buf, "%-30s\r\n", filename_buff);
                        PROCESS_MORE_FUNC(tmp_buf);
                    }
                }
            }
        }
    }
    else              /*some file*/
    {
        memset(&file_attr, 0, sizeof(FS_File_Attr_T));
        strcpy((char *)file_attr.file_name, filename);
        file_attr.file_type_mask = FS_FILE_TYPE_MASK(filetype);

        if(FS_GetFileInfo(unit,&file_attr) == FS_RETURN_OK)
        {
            if(is_startup_only && file_attr.startup_file == FALSE)
                return line_num;

            SYS_TIME_ConvertSecondsToDateTime(file_attr.create_time, &year, &month, &day, &hour, &minute, &second);

            if (strlen((char*)file_attr.file_name) <= FILENAME_LEN_IN_ONE_LINE)
            {
                sprintf(tmp_buf, "%-30s %-7s %-4c    %04d-%02d-%02d %02d:%02d:%02d  %11lu\r\n", file_attr.file_name,
                    file_attr.file_type == FS_FILE_TYPE_DIAG? "Boot-Rom Image": (file_attr.file_type == FS_TYPE_OPCODE_FILE_TYPE? "OpCode" : "Config"),
                    file_attr.startup_file == TRUE? 'Y' : 'N',
                    year, month, day, hour, minute, second,
                    (unsigned long)file_attr.file_size);
                PROCESS_MORE_FUNC(tmp_buf);
        }
        else
        {
                end_of_1st_line = FALSE;
                index = 0;
                filename_len = strlen((char*)file_attr.file_name);

                while (filename_len > 0)
                {
                    if (filename_len >= FILENAME_LEN_IN_ONE_LINE)
                    {
                        memcpy(filename_buff, file_attr.file_name + index, FILENAME_LEN_IN_ONE_LINE);
                        filename_len -= FILENAME_LEN_IN_ONE_LINE;
                        index += FILENAME_LEN_IN_ONE_LINE;
                    }
                    else
                    {
                        memcpy(filename_buff, file_attr.file_name + index, filename_len);
                        filename_buff[filename_len] = '\0';
                        filename_len -= filename_len;
                    }
                    if (end_of_1st_line != TRUE)
                    {
                        sprintf(tmp_buf, "%-30s %-7s %-4c    %04d-%02d-%02d %02d:%02d:%02d  %11lu\r\n", filename_buff,
                            file_attr.file_type == FS_FILE_TYPE_DIAG? "Boot-Rom Image": (file_attr.file_type == FS_TYPE_OPCODE_FILE_TYPE? "OpCode" : "Config"),
                            file_attr.startup_file == TRUE? 'Y' : 'N',
                            year, month, day, hour, minute, second,
                            (unsigned long)file_attr.file_size);

                        PROCESS_MORE_FUNC(tmp_buf);
                        end_of_1st_line = TRUE;
                    }
                    else
                    {
                        sprintf(tmp_buf, "%-30s\r\n", filename_buff);
                        PROCESS_MORE_FUNC(tmp_buf);
                    }
                }
            }
        }
        else
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            PROCESS_MORE_FUNC(MSG_NO_SUCH_FILE);
#endif
        }
    }
    return line_num;
}

static void copy_from_file_to_file(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    COPY_FILE_ServerInfo_T server_info;

    BOOL_T is_startup;

    memset(&server_info, 0, sizeof(server_info));

    /* must be configuration only
     */
    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_CONFIG, src_filename, sizeof(src_filename)))
    {
        return;
    }

    if (FALSE == is_file_existed(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, src_filename, &is_startup))
    {
        CLI_LIB_PrintStr(MSG_NO_SUCH_FILE);
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_CONFIG, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    if (TRUE == is_default_config_file(dst_filename))
    {
        CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED);
        return;
    }

    if (TRUE == is_file_existed(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, dst_filename, &is_startup))
    {
        if (TRUE == is_startup)
        {
            CLI_LIB_PrintStr("This configuration file already exists and is set as startup, replace it <y/n>? ");
        }
        else
        {
            CLI_LIB_PrintStr("This configuration file already exists, replace it <y/n>? ");
        }

        if (FALSE == input_decision())
        {
            return;
        }
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        XFER_MGR_LOCAL_TO_LOCAL,
        XFER_MGR_REMOTE_SERVER_NONE,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
}

static void copy_from_file_to_remote(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    COPY_FILE_ServerInfo_T server_info;

    CLI_API_FILE_TYPE_T available_list[] = { COPY_FILE_TYPE_CONFIG, COPY_FILE_TYPE_RUNTIME };
    const UI32_T AVAILABLE_LIST_LEN = sizeof(available_list) / sizeof(available_list[0]);
    FS_File_Type_T file_type;

    BOOL_T is_startup;

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_file_type(available_list, AVAILABLE_LIST_LEN, &file_type))
    {
        CLI_LIB_PrintStr(MSG_INVALID_INPUT);
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_GENERAL, src_filename, sizeof(src_filename)))
    {
        return;
    }

    if (FALSE == is_file_existed(DUMMY_DRIVE, file_type, src_filename, &is_startup))
    {
        CLI_LIB_PrintStr(MSG_NO_SUCH_FILE);
        return;
    }

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_GENERAL, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        file_type,
        XFER_MGR_LOCAL_TO_REMOTE,
        server_type,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
}

static void copy_from_file_to_running_cfg(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};

    UI8_T *xfer_buf;
    UI32_T xfer_buf_len;

    BOOL_T is_startup;

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_CONFIG, src_filename, sizeof(src_filename)))
    {
        return;
    }

    if (FALSE == is_file_existed(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, src_filename, &is_startup))
    {
        CLI_LIB_PrintStr(MSG_NO_SUCH_FILE);
        return;
    }

    xfer_buf = BUFFER_MGR_Allocate();
    if (NULL == xfer_buf)
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_ALLOCATE_RESOURCE);
        return;
    }

    strcpy((char *)xfer_buf, CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD);

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    if (FS_RETURN_OK != XFER_PMGR_LocalToStream(
        &user_info,
        (UI8_T *)src_filename,
        xfer_buf + strlen(CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD),
        &xfer_buf_len,
        (SYS_ADPT_MAX_FILE_SIZE - strlen(CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD))))
    {
        BUFFER_MGR_Free(xfer_buf);
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_FROM_LOCAL_TO_RUNNING_CFG);
        return;
    }

    if (FALSE == confirm_to_reset())
    {
        memset(xfer_buf, 0, strlen(CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD));
        return;
    }

    notify_to_enter_transition_mode(xfer_buf);
}

static void copy_from_file_to_startup_cfg(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    COPY_FILE_ServerInfo_T server_info;

    BOOL_T is_startup;
    BOOL_T is_startup_cfg_need_to_change;

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_CONFIG, src_filename, sizeof(src_filename)))
    {
        return;
    }

    if (FALSE == is_file_existed(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, src_filename, &is_startup))
    {
        CLI_LIB_PrintStr(MSG_NO_SUCH_FILE);
        return;
    }

    if (FALSE == input_startup_file_name(dst_filename, sizeof(dst_filename), &is_startup_cfg_need_to_change))
    {
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        XFER_MGR_LOCAL_TO_STARTUP,
        XFER_MGR_REMOTE_SERVER_NONE,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    if (FALSE == waiting_xfer_result())
    {
        return;
    }

    if (TRUE == is_startup_cfg_need_to_change)
    {
        sync_config_file(dst_filename);
    }
}

static void copy_from_file_to_unit(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};

    UI32_T max_unit_num;
    UI32_T unit_id = 0;
    char unit_id_buf[2] = {0};

    CLI_API_FILE_TYPE_T available_list[] = { COPY_FILE_TYPE_CONFIG, COPY_FILE_TYPE_RUNTIME };
    const UI32_T AVAILABLE_LIST_LEN = sizeof(available_list) / sizeof(available_list[0]);
    FS_File_Type_T file_type;
    BOOL_T is_startup;

    max_unit_num = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
    CLI_LIB_PrintStr_1("Copy to which unit: <1-%lu>: ", (unsigned long)max_unit_num);
    CLI_PARS_ReadLine(unit_id_buf, sizeof(unit_id_buf), TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    unit_id = atoi(unit_id_buf);
    if (   (unit_id_buf[0] == 0)
        || (unit_id <= 0)
        || (unit_id > max_unit_num)
        || (FALSE == is_current_unit(unit_id)))
    {
        CLI_LIB_PrintStr(MSG_INVALID_INPUT);
        return;
    }

    if (FALSE == input_file_type(available_list, AVAILABLE_LIST_LEN, &file_type))
    {
        CLI_LIB_PrintStr(MSG_INVALID_INPUT);
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_GENERAL, src_filename, sizeof(src_filename)))
    {
        return;
    }

    if (FALSE == is_file_existed(DUMMY_DRIVE, file_type, src_filename, &is_startup))
    {
        CLI_LIB_PrintStr(MSG_NO_SUCH_FILE);
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_GENERAL, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    if (   (FS_FILE_TYPE_CONFIG == file_type)
        && (TRUE == is_default_config_file(dst_filename)))
    {
        CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_LocalToUnit(
        &user_info,
        unit_id,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        file_type,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
}

static void copy_from_remote_to_file(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    COPY_FILE_ServerInfo_T server_info;

    CLI_API_FILE_TYPE_T available_list[] = {
        COPY_FILE_TYPE_CONFIG
        ,COPY_FILE_TYPE_RUNTIME
#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
        ,COPY_FILE_TYPE_TOTAL
#endif /* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */
#if (SYS_CPNT_XFER_DOWNLOAD_DIAG == TRUE)
        ,COPY_FILE_TYPE_DIAG
#endif /* #if (SYS_CPNT_XFER_DOWNLOAD_DIAG == TRUE) */
        ,COPY_FILE_TYPE_LICENSE
        };

    const UI32_T AVAILABLE_LIST_LEN = sizeof(available_list) / sizeof(available_list[0]);
    FS_File_Type_T file_type;

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    if (FALSE == input_file_type(available_list, AVAILABLE_LIST_LEN, &file_type))
    {
        CLI_LIB_PrintStr(MSG_INVALID_INPUT);
        return;
    }

#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
    if (FS_FILE_TYPE_TOTAL == file_type)
    {
        CLI_LIB_PrintStr("Are you sure you want to upgrade loader image (y/n)? ");

        if (FALSE == input_decision())
        {
            return;
        }
    }
#endif /* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_GENERAL, src_filename, sizeof(src_filename)))
    {
        return;
    }

#if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE)
    if (FS_FILE_TYPE_TOTAL == file_type)
    {
        dst_filename[0] = 'L';
        dst_filename[1] = '\0';
    }
    else
#endif /* #if (SYS_CPNT_XFER_DOWNLOAD_LOADER == TRUE) */
    if (FS_FILE_TYPE_LICENSE == file_type)
    {
        strncpy(dst_filename, SYS_ADPT_LICENSE_FILE_NAME, XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME);
    }
    else
    {
        if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_GENERAL, dst_filename, sizeof(dst_filename)))
        {
            return;
        }

        if (TRUE == is_default_config_file(dst_filename))
        {
            CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED);
            return;
        }
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        file_type,
        XFER_MGR_REMOTE_TO_LOCAL,
        server_type,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
}

static void copy_from_remote_to_certificate(XFER_MGR_RemoteServer_T server_type)
{
#if (SYS_CPNT_HTTP_UI == TRUE)
#if (SYS_CPNT_HTTPS == TRUE)
    COPY_FILE_ServerInfo_T server_info;
    HTTP_DownloadCertificateEntry_T certificate_entry;

    memset(&server_info, 0, sizeof(server_info));
    memset(&certificate_entry, 0, sizeof(certificate_entry));

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    memcpy(&certificate_entry.tftp_server, &server_info.ip_address, sizeof(certificate_entry.tftp_server));

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_CERTIFICATE,
                                (char *)certificate_entry.tftp_cert_file,
                                sizeof(certificate_entry.tftp_cert_file)))
    {
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_PRIVATE_KEY,
                                (char *)certificate_entry.tftp_private_file,
                                sizeof(certificate_entry.tftp_private_file)))
    {
        return;
    }

    if (FALSE == input_password((char *)certificate_entry.tftp_private_password,
                                sizeof(certificate_entry.tftp_private_password)))
    {
        return;
    }

    if (HTTP_PMGR_Set_Tftp_Ip(&server_info.ip_address) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set TFTP address.");
        return;
    }

    certificate_entry.cookie = (void *) CLI_TASK_GetMyWorkingArea();

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (HTTP_PMGR_Get_Certificate_From_Tftp(&certificate_entry) != TRUE)
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();

#endif /* #if (SYS_CPNT_HTTPS == TRUE) */
#endif /* #if (SYS_CPNT_HTTP_UI == TRUE) */
}

static void copy_from_remote_to_public_key(XFER_MGR_RemoteServer_T server_type)
{
#if (SYS_CPNT_SSH2 == TRUE)
    XFER_MGR_UserInfo_T  user_info;
    COPY_FILE_ServerInfo_T server_info;
    UI32_T key_type = KEY_TYPE_NONE;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    if (FALSE == input_public_key_type(&key_type))
    {
        CLI_LIB_PrintStr(MSG_INVALID_INPUT);
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_PUBLIC_KEY, src_filename, sizeof(src_filename)))
    {
        return;
    }

    if (FALSE == input_username(publickey_username, sizeof(publickey_username)))
    {
        CLI_LIB_PrintStr(MSG_NO_INVALID_USERNAME);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        key_type,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CERTIFICATE,
        XFER_MGR_REMOTE_TO_PUBLICKEY,
        server_type,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_PUBLIC_KEY_FROM_REMOTE);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
#endif /* #if (SYS_CPNT_SSH2 == TRUE) */
}

static void copy_from_remote_to_running_cfg(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME + 1] = {0};

    COPY_FILE_ServerInfo_T server_info;

    UI8_T *xfer_buf;

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_CONFIG, src_filename, sizeof(src_filename)))
    {
        return;
    }

    xfer_buf = BUFFER_MGR_Allocate();
    if (NULL == xfer_buf)
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_ALLOCATE_RESOURCE);
        return;
    }

    /* should modify to give magic word in xfer later
     */
    strcpy((char *)xfer_buf, CLI_MGR_RUNTIME_PROVISION_MAGIC_WORD);

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_RemoteToStream(
        &user_info,
        server_type,
        &server_info.ip_address,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        xfer_buf + strlen(CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD),
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0,
        (SYS_ADPT_MAX_FILE_SIZE - strlen(CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD))))
    {
        BUFFER_MGR_Free(xfer_buf);
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_RUNNING_CFG_FROM_REMOTE);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    if (FALSE == waiting_xfer_result())
    {
        /* erase signature
         */
        memset(xfer_buf, 0, strlen(CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD));
        return;
    }

    if (FALSE == confirm_to_reset())
    {
        memset(xfer_buf, 0, strlen(CLI_DEF_RUNTIME_PROVISION_MAGIC_WORD));
        return;
    }

    notify_to_enter_transition_mode(xfer_buf);
}

static void copy_from_remote_to_add_run(XFER_MGR_RemoteServer_T server_type)
{
#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
    CLI_TASK_WorkingArea_T *ctrl_p = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();
    XFER_MGR_UserInfo_T  user_info;
    UI32_T  src_oper_type;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME + 1] = {0};
    COPY_FILE_ServerInfo_T server_info;

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_CONFIG, src_filename, sizeof(src_filename)))
    {
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (TRUE != XFER_PMGR_SetFileCopySrcFileName((UI8_T *)src_filename))
    {
        CLI_LIB_PrintStr("Failed to set source file name.\r\n");
        return;
    }

    switch (server_type)
    {
        case XFER_MGR_REMOTE_SERVER_TFTP:
            src_oper_type = VAL_fileCopySrcOperType_tftp;
            break;

        case XFER_MGR_REMOTE_SERVER_FTP:
            src_oper_type = VAL_fileCopySrcOperType_ftp;
            break;

        case XFER_MGR_REMOTE_SERVER_SFTP:
            src_oper_type = VAL_fileCopySrcOperType_sftp;
            break;

        case XFER_MGR_REMOTE_SERVER_FTPS:
            src_oper_type = VAL_fileCopySrcOperType_ftps;
            break;

        default:
            return;
    }

    if (TRUE != XFER_PMGR_SetFileCopySrcOperType(src_oper_type))
    {
        CLI_LIB_PrintStr("Failed to set source operation type.\r\n");
        return;
    }

    /* for tftp, username and password are unnecessary
     */
    if (server_type != XFER_MGR_REMOTE_SERVER_TFTP)
    {
        if (TRUE != XFER_PMGR_SetFileCopyServerUserName((UI8_T *)server_info.login_username))
        {
            CLI_LIB_PrintStr("Failed to set ftp username.\r\n");
            return;
        }

        if (TRUE != XFER_PMGR_SetFileCopyServerPassword((UI8_T *)server_info.login_password))
        {
            CLI_LIB_PrintStr("Failed to set ftp password.\r\n");
            return;
        }
    }

    if (TRUE != XFER_PMGR_SetFileCopyDestOperType(VAL_fileCopyDestOperType_addRunningCfg))
    {
        CLI_LIB_PrintStr("Failed to set destination operation type.\r\n");
        return;
    }

    if (TRUE != XFER_PMGR_SetFileCopyFileType(VAL_fileCopyFileType_config))
    {
        CLI_LIB_PrintStr("Failed to set copy file type.\r\n");
        return;
    }

    if (TRUE != XFER_PMGR_SetFileCopyTftpServer(&server_info.ip_address))
    {
        CLI_LIB_PrintStr("Failed to set server IP address.\r\n");
        return;
    }

    if (TRUE != XFER_PMGR_SetFileCopyAction(&user_info,
                                            VAL_fileCopyAction_copy,
                                            (void *)ctrl_p,
                                            SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
                                            CLI_MGR_XferCopy_Callback))
    {
        CLI_LIB_PrintStr("Failed to set file copy action.\r\n");
        return;
    }

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_LIB_Print_CopyInProgressSign();

#else
    CLI_LIB_PrintStr("No support CLI add-to-running-config from remote server.\r\n");
#endif
}

static void copy_from_remote_to_startup_cfg(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    COPY_FILE_ServerInfo_T server_info;

    BOOL_T is_startup_cfg_need_to_change;

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_CONFIG, src_filename, sizeof(src_filename)))
    {
        return;
    }

    if (FALSE == input_startup_file_name(dst_filename, sizeof(dst_filename), &is_startup_cfg_need_to_change))
    {
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        XFER_MGR_REMOTE_TO_STARTUP,
        server_type,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    if (FALSE == waiting_xfer_result())
    {
        return;
    }

    if (TRUE == is_startup_cfg_need_to_change)
    {
        sync_config_file(dst_filename);
    }
}

static void copy_from_running_cfg_to_file(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};

    UI8_T *xfer_buf;

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_CONFIG, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    if (TRUE == is_default_config_file(dst_filename))
    {
        CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED);
        return;
    }

    if (FS_RETURN_OK != FS_GenericFilenameCheck((UI8_T *)dst_filename, FS_FILE_TYPE_CONFIG))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_RUNNING_CFG_TO_LOCAL);
        return;
    }

    xfer_buf = BUFFER_MGR_Allocate();
    if (NULL == xfer_buf)
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_ALLOCATE_RESOURCE);
        return;
    }

    if (CLI_MGR_RUNCFG_RETURN_NO_ENOUGH_MEMORY == CLI_MGR_Get_RunningCfg(xfer_buf, SYS_ADPT_MAX_FILE_SIZE))
    {
        BUFFER_MGR_Free(xfer_buf);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_StreamToLocal_WriteRunning(
        &user_info,
        (UI8_T *)dst_filename,
        FS_FILE_TYPE_CONFIG,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        xfer_buf,
        0))
    {
        BUFFER_MGR_Free(xfer_buf);
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_RUNNING_CFG_TO_LOCAL);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
}

static void copy_from_running_cfg_to_remote(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME + 1] = {0};

    COPY_FILE_ServerInfo_T server_info;

    UI8_T *xfer_buf;

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_GENERAL, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    if (FALSE == XFER_PMGR_StreamToRemote_Check(&server_info.ip_address,
        (UI8_T *)dst_filename,
        FS_FILE_TYPE_CONFIG,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_RUNNING_CFG_TO_REMOTE);
        return;
    }

    xfer_buf = BUFFER_MGR_Allocate();
    if (NULL == xfer_buf)
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_ALLOCATE_RESOURCE);
        return;
    }

    if (CLI_MGR_RUNCFG_RETURN_NO_ENOUGH_MEMORY == CLI_MGR_Get_RunningCfg(xfer_buf, SYS_ADPT_MAX_FILE_SIZE))
    {
        BUFFER_MGR_Free(xfer_buf);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_StreamToRemote_Write(
        &user_info,
        server_type,
        &server_info.ip_address,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (UI8_T *)dst_filename,
        FS_FILE_TYPE_CONFIG,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        xfer_buf,
        0))
    {
        BUFFER_MGR_Free(xfer_buf);
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_RUNNING_CFG_TO_REMOTE);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
}

static void copy_from_running_cfg_to_startup_cfg(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    COPY_FILE_ServerInfo_T server_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    BOOL_T is_startup_cfg_need_to_change;

    memset(&server_info, 0, sizeof(server_info));

    if (FALSE == input_startup_file_name(dst_filename, sizeof(dst_filename), &is_startup_cfg_need_to_change))
    {
        return;
    }

    if (FS_RETURN_OK != FS_GenericFilenameCheck((UI8_T *)dst_filename, FS_FILE_TYPE_CONFIG))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_RUNNING_CFG_TO_LOCAL);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        XFER_MGR_STREAM_TO_STARTUP,
        XFER_MGR_REMOTE_SERVER_NONE,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_RUNNING_CFG_TO_LOCAL);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    if (FALSE == waiting_xfer_result())
    {
        return;
    }

    if (TRUE == is_startup_cfg_need_to_change)
    {
        sync_config_file(dst_filename);
    }
}

static void copy_from_startup_cfg_to_file(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    COPY_FILE_ServerInfo_T server_info;

    memset(&server_info, 0, sizeof(server_info));

    if (FS_RETURN_OK != FS_GetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, (UI8_T *)src_filename))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_GET_STARTUP_CONFIG_FILE_NAME);
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_CONFIG, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    if (TRUE == is_default_config_file(dst_filename))
    {
        CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        XFER_MGR_STARTUP_TO_LOCAL,
        server_type,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_FILE);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
}

static void copy_from_startup_cfg_to_remote(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_REMOTE_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    COPY_FILE_ServerInfo_T server_info;

    memset(&server_info, 0, sizeof(server_info));

    if (FS_RETURN_OK != FS_GetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, (UI8_T *)src_filename))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_GET_STARTUP_CONFIG_FILE_NAME);
        return;
    }

    if (FALSE == input_remote_server_info(server_type, &server_info))
    {
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_GENERAL, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        XFER_MGR_STARTUP_TO_REMOTE,
        server_type,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
         CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_FROM_LOCAL_TO_REMOTE);
         CLI_MGR_Set_XferProgressStatus(FALSE);
         CLI_MGR_Set_XferProgressResult(FALSE);
         return;
    }

    waiting_xfer_result();
}

static void copy_from_startup_cfg_to_running_cfg(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    COPY_FILE_ServerInfo_T  server_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN + 1] = {0};

    memset(&server_info, 0, sizeof(server_info));

    if (FS_RETURN_OK != FS_GetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, (UI8_T *)src_filename))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_GET_STARTUP_CONFIG_FILE_NAME);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        FS_FILE_TYPE_CONFIG,
        XFER_MGR_STARTUP_TO_STREAM,
        XFER_MGR_REMOTE_SERVER_NONE,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_COPY_FROM_LOCAL_TO_RUNNING_CFG);
        return;
    }

    waiting_xfer_result();
}

static void copy_from_unit_to_file(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};
    char dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME + 1] = {0};

    UI32_T max_unit_num;
    UI32_T unit_id = 0;
    char unit_id_buf[2] = {0};

    CLI_API_FILE_TYPE_T available_list[] = { COPY_FILE_TYPE_CONFIG, COPY_FILE_TYPE_RUNTIME };
    const UI32_T AVAILABLE_LIST_LEN = sizeof(available_list) / sizeof(available_list[0]);
    FS_File_Type_T file_type;
    BOOL_T is_startup;

    max_unit_num = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
    CLI_LIB_PrintStr_1("Copy from which unit: <1-%lu>: ", (unsigned long)max_unit_num);
    CLI_PARS_ReadLine(unit_id_buf, sizeof(unit_id_buf), TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    unit_id = atoi(unit_id_buf);
    if (   (unit_id_buf[0] == 0)
        || (unit_id <= 0)
        || (unit_id > max_unit_num)
        || (FALSE == is_current_unit(unit_id)))
    {
        CLI_LIB_PrintStr(MSG_INVALID_INPUT);
        return;
    }

    if (FALSE == input_file_type(available_list, AVAILABLE_LIST_LEN, &file_type))
    {
        CLI_LIB_PrintStr(MSG_INVALID_INPUT);
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_GENERAL, src_filename, sizeof(src_filename)))
    {
        return;
    }

    if (FALSE == is_file_existed(unit_id, file_type, src_filename, &is_startup))
    {
        CLI_LIB_PrintStr(MSG_NO_SUCH_FILE);
        return;
    }

    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_GENERAL, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    if (   (FS_FILE_TYPE_CONFIG == file_type)
        && (TRUE == is_default_config_file(dst_filename)))
    {
        CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_UnitToLocal(
        &user_info,
        unit_id,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        file_type,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();
}

static BOOL_T waiting_xfer_result(void)
{
    #define WORST_COUNT 20000 /*prevent from bug*/
    #define ERASE_CHAR(CHAR_NUM)  { \
        UI8_T i; \
        for(i=0; i<CHAR_NUM; i++) \
            CLI_LIB_PrintStr(BACK); \
    }

    UI32_T sign_count = 0;
    XFER_MGR_FileCopyStatus_T previous_callback_status = 0xffff;

    while (1)
    {
        BOOL_T Is_Xfer_In_Progress;
        XFER_MGR_FileCopyStatus_T callback_status = 0;

        CLI_MAIN_HandleMGR_Thread(SYSFUN_TIMEOUT_NOWAIT, 0 , NULL);
        CLI_MGR_Get_XferCallBackStatus((UI32_T *)&callback_status);
        CLI_MGR_Get_XferProgressStatus(&Is_Xfer_In_Progress);

        if (!Is_Xfer_In_Progress)
        {
            BOOL_T flag;
            CLI_MGR_Get_XferProgressResult(&flag);

            if (flag == TRUE)
                return TRUE;
            else
                return FALSE;
        }

        switch(sign_count%4)
        {
            case 0:
            default:
                CLI_LIB_PrintStr("-");
                CLI_LIB_PrintStr("\r");
                break;

            case 1:
                CLI_LIB_PrintStr("/");
                CLI_LIB_PrintStr("\r");
                break;

            case 2:
                CLI_LIB_PrintStr("-");
                CLI_LIB_PrintStr("\r");
                break;

            case 3:
                CLI_LIB_PrintStr("\\");
                CLI_LIB_PrintStr("\r");
                break;
        }

        if (previous_callback_status != callback_status)
        {
            previous_callback_status = callback_status;

            switch (callback_status)
            {
                case XFER_MGR_FILE_COPY_COMPLETED: /*The final message*/
                    //CLI_MGR_Set_XferProgressStatus(FALSE);
                    //CLI_MGR_Set_XferCallBackStatus(0);
                    break;

                case XFER_MGR_FILE_COPY_FILE_NOT_FOUND:
                    CLI_LIB_PrintStr("File not found.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_SERVER_PERMISSION_DENIED:
                    CLI_LIB_PrintStr("Server permission denied.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_STORAGE_FULL:
                    CLI_LIB_PrintStr("Storage full.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_ILLEGAL_OPERATION:
                    CLI_LIB_PrintStr("Illegal operation.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_UNKNOWN_TRANSFER_ID:
                    CLI_LIB_PrintStr("Unknown transfer id.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_FILE_EXISTED:
                    CLI_LIB_PrintStr("File existed.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_NO_SUCH_USER:
                    CLI_LIB_PrintStr("No such user.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TIMEOUT:
                    CLI_LIB_PrintStr("Timeout.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_SEND_ERROR:
                    CLI_LIB_PrintStr("Send error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_RECEIVE_ERROR:
                    CLI_LIB_PrintStr("Receive error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_SOCKET_OPEN_ERROR:
                    CLI_LIB_PrintStr("Socket open error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_SOCKET_BIND_ERROR:
                    CLI_LIB_PrintStr("Socket bind error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_TFTP_USER_CANCELED:
                    CLI_LIB_PrintStr("User canceled operation.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_SET_STARTUP_ERROR:
                    CLI_LIB_PrintStr("Set startup file error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_FILE_SIZE_EXCEED:
                    CLI_LIB_PrintStr("File size exceed.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_MAGIC_WORD_ERROR:
                    CLI_LIB_PrintStr("Failed to set to running-config word.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_SUCCESS:
                    CLI_LIB_PrintStr("Success.\r\n");
                    CLI_MGR_Set_XferProgressResult(TRUE);
                    break;

                case XFER_MGR_FILE_COPY_ERROR:
                    CLI_LIB_PrintStr("Error.\r\n");
                    CLI_MGR_Set_XferProgressResult(FALSE);
                    break;

                case XFER_MGR_FILE_COPY_HEADER_CHECKSUM_ERROR:
                    CLI_LIB_PrintStr("Header checksum error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_IMAGE_CHECKSUM_ERROR:
                    CLI_LIB_PrintStr("Image checksum error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_IMAGE_TYPE_ERROR:
                    CLI_LIB_PrintStr("Image type error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROJECT_ID_ERROR:
                    CLI_LIB_PrintStr("Project ID error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_BUSY:
                    CLI_LIB_PrintStr("Busy.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PARA_ERROR:
                    CLI_LIB_PrintStr("Parameter error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_WRITE_FLASH_FINISH:
                    CLI_LIB_PrintStr("Flash programming completed.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_WRITE_FLASH_ERR:
                    CLI_LIB_PrintStr("Flash programming error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_WRITE_FLASH_PROGRAMMING:
                    CLI_LIB_PrintStr("Flash programming started.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_READ_FILE_ERROR:
                    CLI_LIB_PrintStr("Read file error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_UNKNOWN:
                    CLI_LIB_PrintStr("Unknown error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROGRESS_UNIT1:
                    CLI_LIB_PrintStr("Synchronizing to Unit1.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROGRESS_UNIT2:
                    CLI_LIB_PrintStr("Synchronizing to Unit 2.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROGRESS_UNIT3:
                    CLI_LIB_PrintStr("Synchronizing to Unit 3.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROGRESS_UNIT4:
                    CLI_LIB_PrintStr("Synchronizing to Unit 4.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROGRESS_UNIT5:
                    CLI_LIB_PrintStr("Synchronizing to Unit 5.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROGRESS_UNIT6:
                    CLI_LIB_PrintStr("Synchronizing to Unit 6.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROGRESS_UNIT7:
                    CLI_LIB_PrintStr("Synchronizing to Unit 7.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_PROGRESS_UNIT8:
                    CLI_LIB_PrintStr("Synchronizing to Unit 8.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_CONNECT_ERROR:
                    CLI_LIB_PrintStr("Failed to connect to server.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_SERVER_NOT_ACCEPT_PROVIDED_CIPHERS:
                    CLI_LIB_PrintStr("Server did not accept provided ciphers.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_SERVER_NOT_SUPPORT_FTPS:
                    CLI_LIB_PrintStr("Server does not support FTPS.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_DATA_CONNECTION_OPEN_ERROR:
                    CLI_LIB_PrintStr("Failed to open data connection.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_SERVER_NOT_IN_SERVICE:
                    CLI_LIB_PrintStr("Server not in service.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_LOG_IN_ERROR:
                    CLI_LIB_PrintStr("Failed to log in.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_FILE_UNAVAILABLE:
                    CLI_LIB_PrintStr("File unavailable.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_INVALID_FILE_NAME:
                    CLI_LIB_PrintStr("Invalid file name.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR:
                    CLI_LIB_PrintStr("Unclassified error.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_WRITE_LOADER_TO_FLASH:
                    CLI_LIB_PrintStr("Warning!! The file system is updating loader image to FLASH.\r\n");
                    CLI_LIB_PrintStr("Please DO NOT turn off the power.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_FILE_NUM_EXCEED:
                    CLI_LIB_PrintStr("The number of files of the given type exceeds the maximum number.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_SAME_VERSION:
                    CLI_LIB_PrintStr("Same version number. Not updated.\r\n");
                    break;

                case XFER_MGR_FILE_COPY_OP_CODE_VERSION_NOT_SUPPORTED_BY_LICENSE:
                    CLI_LIB_PrintStr("Opcode version is not supported by license.\r\n");
                    break;
            }
        }

        SYSFUN_Sleep(10);

        if (sign_count++ > WORST_COUNT)
            break;
    }
    CLI_LIB_PrintStr("\r\n");
    return TRUE;
}

static BOOL_T is_file_existed(UI32_T unit, UI32_T file_type, char *filename, BOOL_T *is_startup)
{
    FS_File_Attr_T file_attr = {0};

    file_attr.file_type_mask = FS_FILE_TYPE_MASK(file_type);
    strcpy((char *)file_attr.file_name, filename);

    if (FS_RETURN_OK != FS_GetFileInfo(unit, &file_attr))
    {
        return FALSE;
    }

    *is_startup = (TRUE == file_attr.startup_file) ? TRUE : FALSE;

    return TRUE;
}

static BOOL_T input_file_type(CLI_API_FILE_TYPE_T *available_list, const UI32_T AVAILABLE_LIST_LEN, FS_File_Type_T *input_value)
{
    UI32_T count = 0;
    UI32_T i;
    UI32_T choice;
    char buf[2] = {0};

    CLI_LIB_PrintStr("Choose file type: \r\n");
    CLI_LIB_PrintStr(" ");

    for (i = 0; i < AVAILABLE_LIST_LEN; i++)
    {
        count++;

        CLI_LIB_PrintStr_2("%lu. %s", (unsigned long)count, available_list[i].name);

        if (i == (AVAILABLE_LIST_LEN - 1))
        {
            CLI_LIB_PrintStr(": ");
        }
        else
        {
            CLI_LIB_PrintStr("; ");
        }
    }

    CLI_PARS_ReadLine(buf, sizeof(buf), TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);
    choice = atoi(buf);

    if (   (0 == buf[0])
        || (choice <= 0)
        || (choice > AVAILABLE_LIST_LEN))
    {
        return FALSE;
    }

    *input_value = available_list[choice - 1].type;

    return TRUE;
}

#if (SYS_CPNT_SSH2 == TRUE)
static BOOL_T input_public_key_type(UI32_T *key_type_p)
{
    UI32_T choice;
    char buf[2] = {0};

    if (NULL == key_type_p)
    {
        return FALSE;
    }

    CLI_LIB_PrintStr("Choose public key type:\r\n");
    CLI_LIB_PrintStr(" 1. RSA:  2. DSA: <1-2>: ");
    CLI_PARS_ReadLine(buf, sizeof(buf), TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    if (0 == buf[0])
    {
        return FALSE;
    }

    choice = atoi(buf);
    switch (choice)
    {
        case 1:
            *key_type_p = KEY_TYPE_RSA;
            break;

        case 2:
            *key_type_p = KEY_TYPE_DSA;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

static BOOL_T input_username(char *username, UI32_T username_len)
{
    if (   (NULL == username)
        || (username_len <= 0))
    {
        return FALSE;
    }

    CLI_LIB_PrintStr("Username: ");
    CLI_PARS_ReadLine(username, username_len, TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    if (strlen(username)==0)
    {
        return FALSE;
    }

    return TRUE;
}
#endif  /* #if (SYS_CPNT_SSH2 == TRUE) */

#if (SYS_CPNT_HTTP_UI == TRUE)
static BOOL_T input_password(char *password, UI32_T password_len)
{
    if (   (NULL == password)
        || (password_len <= 0))
    {
        return FALSE;
    }

    CLI_LIB_PrintStr("Private password: ");
    CLI_PARS_ReadLine(password, password_len, TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    return TRUE;
}
#endif /* #if (SYS_CPNT_HTTP_UI == TRUE) */

static BOOL_T is_default_config_file(char *filename)
{
    return (0 == strcmp(filename, SYS_DFLT_restartConfigFile));
}

static BOOL_T input_startup_file_name(char *filename, UI32_T filename_len, BOOL_T *is_startup_cfg_need_to_change)
{
    char startup_cfg_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME + 1] = {0};

    if (   (FS_RETURN_OK != FS_GetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, (UI8_T *)startup_cfg_filename))
        || ('\0' == startup_cfg_filename[0]))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_GET_STARTUP_CONFIG_FILE_NAME);
        return FALSE;
    }

    if (TRUE == is_default_config_file(startup_cfg_filename))
    {
        CLI_LIB_PrintStr("Startup configuration file name []: ");
    }
    else
    {
        CLI_LIB_PrintStr_1("Startup configuration file name [%s]: ", startup_cfg_filename);
    }

    CLI_PARS_ReadLine(filename, filename_len, TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    if ('\0' == filename[0])
    {
        if (TRUE == is_default_config_file(startup_cfg_filename))
        {
            CLI_LIB_PrintStr(MSG_INVALID_FILE_NAME);
            return FALSE;
        }

        /* use original startup config
         */
        strcpy(filename, startup_cfg_filename);
    }
    else
    {
        if (TRUE == is_default_config_file(filename))
        {
            CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED);
            return FALSE;
        }

        *is_startup_cfg_need_to_change = (0 == strcmp(filename, startup_cfg_filename)) ? FALSE : TRUE;
    }

    return TRUE;
}

static BOOL_T input_decision()
{
    char choice[2] = {0};

    CLI_PARS_ReadLine(choice, sizeof(choice), TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    switch (choice[0])
    {
        case 'y':
        case 'Y':
            return TRUE;

        case 'n':
        case 'N':
            return FALSE;

        default:
            CLI_LIB_PrintStr(MSG_INVALID_INPUT);
            return FALSE;
    }
}

static BOOL_T input_remote_server_info(XFER_MGR_RemoteServer_T server_type, COPY_FILE_ServerInfo_T *server_info_p)
{
    char server_ip_buf[L_INET_MAX_IPADDR_STR_LEN + 1] = {0};

    char temp_username[MAXSIZE_fileCopyServerUserName + 1] = {0};
    char temp_password[MAXSIZE_fileCopyServerPassword + 1] = {0};

    if (XFER_MGR_REMOTE_SERVER_NONE == server_type)
    {
        return FALSE;
    }

    if (NULL == server_info_p)
    {
        return FALSE;
    }

    switch (server_type)
    {
        case XFER_MGR_REMOTE_SERVER_TFTP:
            CLI_LIB_PrintStr("TFTP server IP address: ");
            break;

        case XFER_MGR_REMOTE_SERVER_FTP:
            CLI_LIB_PrintStr("FTP server IP address: ");
            break;

        case XFER_MGR_REMOTE_SERVER_FTPS:
            CLI_LIB_PrintStr("FTPS server IP address: ");
            break;

        case XFER_MGR_REMOTE_SERVER_SFTP:
            CLI_LIB_PrintStr("SFTP server IP address: ");
            break;

        default:
            return FALSE;
    }

    CLI_PARS_ReadLine(server_ip_buf, sizeof(server_ip_buf), TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       server_ip_buf,
                                                       (L_INET_Addr_T *) &server_info_p->ip_address,
                                                       sizeof(server_info_p->ip_address)))
    {
        switch (server_type)
        {
            case XFER_MGR_REMOTE_SERVER_TFTP:
                CLI_LIB_PrintStr("Invalid TFTP server IP address.\r\n");
                break;

            case XFER_MGR_REMOTE_SERVER_FTP:
                CLI_LIB_PrintStr("Invalid FTP server IP address.\r\n");
                break;

            case XFER_MGR_REMOTE_SERVER_FTPS:
                CLI_LIB_PrintStr("Invalid FTPS server IP address.\r\n");
                break;

            case XFER_MGR_REMOTE_SERVER_SFTP:
                CLI_LIB_PrintStr("Invalid SFTP server IP address.\r\n");
                break;

            default:
                break;
        }

        return FALSE;
    }

    /* for tftp, username and password are unnecessary
     */
    if (XFER_MGR_REMOTE_SERVER_TFTP == server_type)
    {
        return TRUE;
    }

    strncpy(server_info_p->login_username, XFER_MGR_DFLT_USERNAME, MAXSIZE_fileCopyServerUserName);
    strncpy(server_info_p->login_password, XFER_MGR_DFLT_PASSWORD, MAXSIZE_fileCopyServerPassword);

    CLI_LIB_PrintStr("User [");
    CLI_LIB_PrintStr(server_info_p->login_username);
    CLI_LIB_PrintStr("]: ");
    CLI_PARS_ReadLine(temp_username, sizeof(temp_username), TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    CLI_LIB_PrintStr("Password: ");
    CLI_PARS_ReadLine(temp_password, sizeof(temp_password), TRUE, TRUE);
    CLI_LIB_PrintNullStr(1);

    if (strlen(temp_username) > 0)
    {
        strcpy(server_info_p->login_username, temp_username);
    }

    if (strlen(temp_password) > 0)
    {
        strcpy(server_info_p->login_password, temp_password);
    }

    return TRUE;
}

static BOOL_T is_current_unit(UI32_T unit_id)
{
    UI32_T i;

    for (i = 0; STKTPLG_POM_GetNextUnit(&i);)
    {
        if (i == unit_id)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL_T parse_copy_type(char *arg, COPY_T *copy_type, XFER_MGR_RemoteServer_T *remote_type)
{
    *remote_type = XFER_MGR_REMOTE_SERVER_NONE;

    switch (arg[0])
    {
#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
        case 'a':
        case 'A':
            /* add-to-running-config
             */
            *copy_type = COPY_TO_CURRENT_RUN;
            break;
#endif  /* #if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE) */

        case 'f':
        case 'F':
            /* file, ftp or ftps
             */
            if (arg[1] == 't' || arg[1] == 'T')
            {
                if (arg[3] == 's' || arg[3] == 'S')
                {
                    *copy_type = COPY_REMOTE;
                    *remote_type = XFER_MGR_REMOTE_SERVER_FTPS;
                }
                else
                {
                    *copy_type = COPY_REMOTE;
                    *remote_type = XFER_MGR_REMOTE_SERVER_FTP;
                }
            }
            else
            {
                *copy_type = COPY_FILE;
            }
            break;

        case 'h':
        case 'H':
            /* https-certificate
             */
            *copy_type = COPY_CERTIFICATE;
            break;

        case 'p':
        case 'P':
            /* public-key
             */
            *copy_type = COPY_PUBLIC_KEY;
            break;

        case 'r':
        case 'R':
            /* running-config
             */
            *copy_type = COPY_RUNNING_CFG;
            break;

        case 's':
        case 'S':
            /* startup-config or sftp
             */
            if (arg[1] == 'f' || arg[1] == 'F')
            {
                *copy_type = COPY_REMOTE;
                *remote_type = XFER_MGR_REMOTE_SERVER_SFTP;
            }
            else
            {
                *copy_type = COPY_STARTUP_CFG;
            }
            break;

        case 't':
        case 'T':
            /* tftp
             */
            *copy_type = COPY_REMOTE;
            *remote_type = XFER_MGR_REMOTE_SERVER_TFTP;
            break;

        case 'u':
        case 'U':
            /* unit or usbdisk
             */
            if (arg[1] == 's' || arg[1] == 'S')
            {
                *copy_type = COPY_USBDISK;
            }
            else
            {
                *copy_type = COPY_UNIT;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

static void notify_to_enter_transition_mode(UI8_T *xfer_buf)
{
    I32_T offset;

    offset = BUFFER_MGR_GetOffset(xfer_buf);
    CLI_MGR_Set_BuffermgmtOffset(offset);
    CLI_MGR_Set_RuntimeProvisionFlag(TRUE);
    CLI_MGR_Notify_EnterTransitionMode();
}

static BOOL_T confirm_to_reset()
{
    BOOL_T ret = TRUE;

#if (CLI_SHOW_RUNTIME_PROV_WARRING_MSG == TRUE)
    CLI_LIB_PrintStr("\r\nWarning: System configuration will be changed and the system will restart.\r\n");
    CLI_LIB_PrintStr("Press 'y' to continue or any key to abort ... \r\n");
    while (1)
    {
        UI8_T ch;
        UI16_T ret_val;

        ret_val = CLI_IO_GetKey(&ch);

        if (   (ret_val != KEYIN_ERROR)
            && (ret_val != UNKNOWN))
        {
            if (   (ch == 'y')
                || (ch == 'Y'))
            {
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }
        }
    }
#endif /* #if (CLI_SHOW_RUNTIME_PROV_WARRING_MSG == TRUE) */

    return ret;
}

static BOOL_T input_filename(char *message, char *filename, UI32_T filename_len)
{
    if (NULL == message)
    {
        return FALSE;
    }

    CLI_LIB_PrintStr(message);

    CLI_PARS_ReadLine(filename, filename_len, TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    if ('\0' == filename[0])
    {
        CLI_LIB_PrintStr(MSG_INVALID_FILE_NAME);
        return FALSE;
    }

    return TRUE;
}

static BOOL_T sync_config_file(char *filename)
{
   UI8_T   unit_list[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK] = {0};
   UI32_T  i = 0;
   UI8_T   master_unit_id;
   BOOL_T  stacking = FALSE;

    if (FS_RETURN_OK != FS_SetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, (UI8_T *)filename))
    {
        CLI_LIB_PrintStr(MSG_FAILED_TO_SET_STARTUP_CONFIG);
        return FALSE;
    }

   STKTPLG_POM_GetMasterUnitId(&master_unit_id);
    /*check current dut in stacking or stand alone mode if stacking syn to slave, if not ,return true*/
   for(i=1; i<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
   {
        if(i == master_unit_id)
            continue;

        if(STKTPLG_POM_UnitExist(i))
        {
            stacking = TRUE;
            break;
        }

   }
   if(!stacking)
        return TRUE;

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_AutoDownLoad(unit_list,
        (UI8_T *)filename,
        (UI8_T *)filename,
        FS_FILE_TYPE_CONFIG,
        TRUE,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        NULL))
   {
       CLI_LIB_PrintStr(MSG_COPY_ERROR);
       CLI_MGR_Set_XferProgressStatus(FALSE);
       CLI_MGR_Set_XferProgressResult(FALSE);
       return FALSE;
   }

   waiting_xfer_result();

   return TRUE;
}

/*gxz add,for mpc8347 usb,2009-04-09,begin*/
static void dir_usbdisk(UI8_T *path)
{
    char printbuf[CLI_DEF_MAX_BUFSIZE] = {0};
    char fullpathname[USB_PATH_LENGTH + 10] = "/mnt/usb/";
    char dirfullpathname[USB_PATH_LENGTH + 10 + 1];
    DIR *dirstream_p;
    struct dirent *file_p;
    struct stat file_stat;
    struct tm *tm_p;
    struct statfs   disk_statfs;
    int len;
    UI64_T usbsize;
    UI64_T Gbyte = 1024 * 1024 * 1024;

    if(TRUE != mount_usbdisk())
    {
        CLI_LIB_PrintStr("Usbdisk is not ready.\r\n");
        return;
    }

    if(NULL != path)
    {
        if('/' == path[0])
        {
            /*path begins with '/'*/
            strncat((char*)fullpathname, (const char *)&path[1], sizeof(fullpathname) - strlen(fullpathname) - 1);
        }
        else
        {
            /*path doesn't begin with '/'*/
            strncat((char*)fullpathname, (const char *)path, sizeof(fullpathname) - strlen(fullpathname) - 1);
        }
    }

    /*file exists?*/
    if(0 != access (fullpathname, F_OK))
    {
        CLI_LIB_PrintStr("No such file or directory.\r\n");
        return;
    }

    if(stat(fullpathname,&file_stat) < 0)
    {
        CLI_LIB_PrintStr("Can't get the information of the file.\r\n");
        return;
    }

    CLI_LIB_PrintStr("Modify Time         Type  Size(byte) File Name\r\n");
    CLI_LIB_PrintStr("------------------- ----- ---------- ------------------------------------------\r\n");

    if(S_ISDIR(file_stat.st_mode))
    {
        /*dir*/

        strncpy(dirfullpathname, fullpathname, sizeof(dirfullpathname) - 1);
        dirfullpathname[sizeof(dirfullpathname) - 1] = '\0';

        len = strlen(dirfullpathname);

        /*full dir name including '/'*/
        if('/' != dirfullpathname[len-1])
        {
            dirfullpathname[len] = '/';
            dirfullpathname[len+1] = 0;
        }

        dirstream_p = opendir(dirfullpathname);
        if(NULL == dirstream_p)
        {
            CLI_LIB_PrintStr("Can't open this directory.\r\n");
            return;
        }

        while( (file_p = readdir (dirstream_p))!= NULL )
        {
            /*skip . */
            if  (0 == strcmp(file_p->d_name,".") )
            {
                    continue;
            }

                /*skip .. */
            if  (0 == strcmp(file_p->d_name,"..") )
            {
                    continue;
            }

            memset(fullpathname,0,sizeof(fullpathname));
            strncpy(fullpathname, dirfullpathname, sizeof(fullpathname) - 1);
            fullpathname[sizeof(fullpathname) - 1] = '\0';
            strncat(fullpathname, file_p->d_name, sizeof(fullpathname) - strlen(fullpathname) - 1);

            if(stat(fullpathname,&file_stat) < 0)
            {
                continue;
            }

            tm_p = localtime(&file_stat.st_mtime);

            if(S_ISDIR(file_stat.st_mode))
            {
                memset(printbuf,0,sizeof(printbuf));
                sprintf(printbuf,"%04d-%02d-%02d %02d:%02d:%02d ",tm_p->tm_year+1900,tm_p->tm_mon+1,tm_p->tm_mday,tm_p->tm_hour,tm_p->tm_min,tm_p->tm_sec);
                CLI_LIB_PrintStr(printbuf);
                CLI_LIB_PrintStr("<dir> ");
                CLI_LIB_PrintStr_1("%10d ",(int)file_stat.st_size);
                dir_usbdisk_filename((char *)(file_p->d_name));
            }
            else
            {
                if(S_ISREG(file_stat.st_mode))
                {
                    memset(printbuf,0,sizeof(printbuf));
                    sprintf(printbuf,"%04d-%02d-%02d %02d:%02d:%02d ",tm_p->tm_year+1900,tm_p->tm_mon+1,tm_p->tm_mday,tm_p->tm_hour,tm_p->tm_min,tm_p->tm_sec);
                    CLI_LIB_PrintStr(printbuf);
                    CLI_LIB_PrintStr("      ");
                    CLI_LIB_PrintStr_1("%10d ",(int)file_stat.st_size);
                    dir_usbdisk_filename((char *)(file_p->d_name));
                }
                else
                {
                    continue;
                }
            }

        }

        closedir(dirstream_p);

    }
    else
    {
        if(S_ISREG(file_stat.st_mode))
        {
            /*regular file*/
            memset(printbuf,0,sizeof(printbuf));
            tm_p = localtime(&file_stat.st_mtime);
            sprintf(printbuf,"%04d-%02d-%02d %02d:%02d:%02d ",tm_p->tm_year+1900,tm_p->tm_mon+1,tm_p->tm_mday,tm_p->tm_hour,tm_p->tm_min,tm_p->tm_sec);
            CLI_LIB_PrintStr(printbuf);
            CLI_LIB_PrintStr("      ");
            CLI_LIB_PrintStr_1("%10d ",(int)file_stat.st_size);
            dir_usbdisk_filename((char *)basename((char *)fullpathname));
        }
        else
        {
            CLI_LIB_PrintStr("Unknow file type.\r\n");
            return;
        }
    }

    /*show free space*/
    memset(&disk_statfs,0,sizeof(disk_statfs));
    CLI_LIB_PrintStr("-------------------------------------------------------------------------------\r\n");
    if(statfs("/mnt/usb/",   &disk_statfs)<0)
    {
        CLI_LIB_PrintStr("Failed to get free space.\r\n");
        return;
    }

    usbsize = (UI64_T) disk_statfs.f_bavail * (UI64_T) disk_statfs.f_bsize;

    if (usbsize>=(2*Gbyte))
    {
        CLI_LIB_PrintStr_1("Total free space: %.3f GByte\r\n",  (double) usbsize/Gbyte);
    }
    else
    {
        CLI_LIB_PrintStr_1("Total free space: %llu Byte\r\n",  (unsigned long long)usbsize);
    }

}

#define FILENAME_LEN_IN_ONE_LINE_USB_DIR 42
static void dir_usbdisk_filename(char *filename)
{
    UI32_T index = 0;
    UI32_T filename_len = 0;
    UI8_T  filename_buff[FILENAME_LEN_IN_ONE_LINE_USB_DIR + 1] = "";
    char   prefix_space[79 - FILENAME_LEN_IN_ONE_LINE_USB_DIR + 1] = ""; /* max table width - FILENAME_LEN_IN_ONE_LINE_USB_DIR + end of string*/

    memset(prefix_space, ' ', sizeof(prefix_space));
    prefix_space[79 - FILENAME_LEN_IN_ONE_LINE_USB_DIR] = '\0';

    if(strlen(filename) <= FILENAME_LEN_IN_ONE_LINE_USB_DIR)
    {
        CLI_LIB_PrintStr_1("%-s\r\n", filename);
    }
    else
    {
        index = 0;
        filename_len = strlen(filename);

        while (filename_len > 0)
        {
            if (index > 0)
            {
                /* Fill prefix space chars when printing file name in 2nd/3rd/4th ... lines.
                 */
                CLI_LIB_PrintStr(prefix_space);
            }
            if (filename_len >= FILENAME_LEN_IN_ONE_LINE_USB_DIR)
            {
                memcpy(filename_buff, filename + index, FILENAME_LEN_IN_ONE_LINE_USB_DIR);
                filename_len -= FILENAME_LEN_IN_ONE_LINE_USB_DIR;
                index += FILENAME_LEN_IN_ONE_LINE_USB_DIR;
            }
            else
            {
                memcpy(filename_buff, filename + index, filename_len);
                filename_buff[filename_len] = '\0';
                filename_len -= filename_len;
            }
            CLI_LIB_PrintStr_1("%-s\r\n", filename_buff);
        }
    }
}

static void copy_from_usbdisk_to_file(XFER_MGR_RemoteServer_T server_type)
{
    XFER_MGR_UserInfo_T  user_info;
    char  dst_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_DEST_FILE_NAME+1]={0};
    char  src_filename[USB_MAXSIZE_DestFile+1]={0};
    char src_fullpathname[USB_MAXSIZE_DestFile+10]="/mnt/usb/";
    char publickey_username[SYS_ADPT_MAX_USER_NAME_LEN +1] = {0};
    int i;
    FS_File_Type_T file_type;
    CLI_API_FILE_TYPE_T available_list[] = {
        COPY_FILE_TYPE_CONFIG
        ,COPY_FILE_TYPE_RUNTIME
    };
    const UI32_T AVAILABLE_LIST_LEN = sizeof(available_list) / sizeof(available_list[0]);
    COPY_FILE_ServerInfo_T server_info;

    memset(&server_info, 0, sizeof(server_info));

    if(TRUE != mount_usbdisk())
    {
        CLI_LIB_PrintStr("Usbdisk is not ready.\r\n");
        return;
    }


      /*file type*/
    if (FALSE == input_file_type(available_list, AVAILABLE_LIST_LEN, &file_type))
    {
        CLI_LIB_PrintStr(MSG_INVALID_INPUT);
        return;
    }
    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_SRC_GENERAL, src_filename, sizeof(src_filename)))
    {
        return;
    }

    /*skip '/' from the beginning of dest_filename*/
    i = 0;
    while('/' == src_filename[i]) i++;
    strncat(src_fullpathname, &src_filename[i], sizeof(src_fullpathname) - strlen(src_fullpathname) - 1);

    /*destination file exists?*/
    if(0 != access (src_fullpathname, F_OK))
    {
        CLI_LIB_PrintStr("No such file.\r\n");
        return;
    }
    if (FALSE == input_filename(MSG_COPY_FILE_TYPE_DST_GENERAL, dst_filename, sizeof(dst_filename)))
    {
        return;
    }

    if (TRUE == is_default_config_file(dst_filename))
    {
        CLI_LIB_PrintStr(MSG_DEFAULT_CONFIG_CANNOT_BE_REPLACED);
        return;
    }

    memset(&user_info, 0, sizeof(user_info));
    get_file_copy_user_info(&user_info);

    CLI_MGR_Set_XferProgressStatus(TRUE);
    CLI_MGR_Set_XferProgressResult(FALSE);

    if (FALSE == XFER_PMGR_CopyFile(
        &user_info,
        publickey_username,
        0,
        &server_info.ip_address,
        (UI8_T *)dst_filename,
        (UI8_T *)src_filename,
        file_type,
        XFER_MGR_USBDISK_TO_LOCAL,
        XFER_MGR_REMOTE_SERVER_NONE,
        (UI8_T *)server_info.login_username,
        (UI8_T *)server_info.login_password,
        (void *)CLI_TASK_GetMyWorkingArea(),
        SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
        0))
    {
        CLI_LIB_PrintStr(MSG_COPY_ERROR);
        CLI_MGR_Set_XferProgressStatus(FALSE);
        CLI_MGR_Set_XferProgressResult(FALSE);
        return;
    }

    waiting_xfer_result();

}

static void copy_from_file_To_usbdisk(XFER_MGR_RemoteServer_T server_type)
{
    char  src_filename[XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME+1]={0};
    char  dest_filename[USB_MAXSIZE_DestFile+1]={0};
    char dest_fullpathname[USB_MAXSIZE_DestFile+10]="/mnt/usb/";
    int i;
    UI32_T file_type;
    char  choice[2] = {0};
    BOOL_T is_startup;

    UI32_T unit_id;
    I8_T *file_buffer = NULL;
    int fd_dst;
    UI32_T read_cnt = 0;
    UI32_T offset = 0;
    UI32_T len = 0;
    UI32_T block_size = USB_FILEOPER_MGR_BLOCKSIZE;

    if(TRUE != mount_usbdisk())
    {
        CLI_LIB_PrintStr("Usbdisk is not ready.\r\n");
        return;
    }


    /*file type*/
    memset(choice,0,sizeof(choice));
    CLI_LIB_PrintStr("Choose file type: \r\n");
    CLI_LIB_PrintStr(" 1. config:  2. opcode: <1-2>: ");
    CLI_PARS_ReadLine( choice, sizeof(choice), TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    if(strlen(choice)!=0)
    {
        switch(choice[0])
        {
            case '1':
                file_type=FS_FILE_TYPE_CONFIG;
            break;

            case '2':
                file_type=FS_TYPE_OPCODE_FILE_TYPE;
            break;

            default:
                CLI_LIB_PrintStr("Invalid input.\r\n");
                return;
        }
    }
    else
    {
        CLI_LIB_PrintStr("Invalid input.\r\n");
        return;
    }

    /*get source file name*/
    CLI_LIB_PrintStr("Source file name: ");
    CLI_PARS_ReadLine( src_filename, XFER_TYPE_MAX_SIZE_OF_LOCAL_SRC_FILE_NAME+1, TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    if(strlen(src_filename)==0)
    {
        CLI_LIB_PrintStr("Invalid file name.\r\n");
        return;
    }
    else if(!is_file_existed(DUMMY_DRIVE, file_type, src_filename, &is_startup))
    {
        CLI_LIB_PrintStr("No such file.\r\n");
        return;
    }

      /*destination file name*/
    CLI_LIB_PrintStr("Destination file name: ");
    CLI_PARS_ReadLine(dest_filename, USB_MAXSIZE_DestFile+1, TRUE, FALSE);
    CLI_LIB_PrintNullStr(1);

    if(strlen(dest_filename)==0)
    {
        CLI_LIB_PrintStr("Invalid file name.\r\n");
        return;
    }

    /*skip '/' from the beginning of dest_filename*/
    i =0;
    while('/' == dest_filename[i]) i++;
    strcat(dest_fullpathname,&dest_filename[i]);

    /*destination file is existed,overwrite?*/
    if(0 == access (dest_fullpathname, F_OK))
    {
        memset(choice,0,sizeof(choice));
        CLI_LIB_PrintStr("This destination file already exist, replace it <y/n>?");
        CLI_PARS_ReadLine( choice, sizeof(choice), TRUE, FALSE);
        CLI_LIB_PrintNullStr(1);
        if ( ('y' != choice[0] )&&('Y' != choice[0] ))
        {

            return;
        }
    }

    /*action*/
#if     1
    unit_id = 1;
#else
    STKTPLG_MGR_GetMyUnitID(&unit_id);
#endif

    file_buffer = BUFFER_MGR_Allocate();
    if(file_buffer == NULL)
    {
        CLI_LIB_PrintStr("Fail to allocate buffer.\r\n");
        return;
    }

    if(FS_ReadFile(unit_id, (UI8_T *)src_filename, (UI8_T *)file_buffer, SYS_ADPT_MAX_FILE_SIZE, &read_cnt) != FS_RETURN_OK)
    {
        CLI_LIB_PrintStr("Fail to read source file.\r\n");
        BUFFER_MGR_Free(file_buffer);
        return;
    }

    fd_dst = open(dest_fullpathname,O_CREAT|O_WRONLY);
    if(fd_dst < 0)
    {
        CLI_LIB_PrintStr("Fail to create destination file.\r\n");
        BUFFER_MGR_Free(file_buffer);
        return;
    }

    if(read_cnt < USB_FILEOPER_MGR_BLOCKSIZE)
    {
        block_size = read_cnt;
    }

    do
    {
        len = write(fd_dst, &(file_buffer[offset]), block_size);
        if(len < 0)
        {
            CLI_LIB_PrintStr("Fail to write destination file.\r\n");
            BUFFER_MGR_Free(file_buffer);
            close(fd_dst);
            return;
        }

        if(len < block_size)
        {
            CLI_LIB_PrintStr("Fail to write destination file.\r\n");
            BUFFER_MGR_Free(file_buffer);
            close(fd_dst);
            return;
        }

        CLI_LIB_PrintStr(".");
        offset += len;

            if((read_cnt - offset) < USB_FILEOPER_MGR_BLOCKSIZE)
            {
                block_size = read_cnt - offset;
            }

    }
    while(offset < read_cnt);

    CLI_LIB_PrintStr("\r\n");

    BUFFER_MGR_Free(file_buffer);
    close(fd_dst);
    CLI_LIB_PrintStr("Success.\r\n");

    return;
}

static BOOL_T mount_usbdisk()
{
/* TBD: this function should be moved to fs.c
 */
#if (SYS_CPNT_FS_USE_SCRIPT_TO_MOUNT_USBDISK == TRUE)
#define MOUNT_USBDISK_SCRIPT_FILENAME "/etc/mount_usbdisk.sh"

#ifdef USBDISK_DEBUG
#define USBDISK_MOUNT_DEBUG_MSG CLI_LIB_Printf
#else
#define USBDISK_MOUNT_DEBUG_MSG(...)
#endif
    struct stat file_stat;
    UI32_T rc;
    int    shell_exit_status;

    /* check that whether the script file exists in rootfs
     */
    if(stat(MOUNT_USBDISK_SCRIPT_FILENAME,&file_stat) < 0)
    {
        /* critical error: show error message
         */
        CLI_LIB_Printf("%s(%d): script file not found.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    rc = SYSFUN_ExecuteSystemShellEx(MOUNT_USBDISK_SCRIPT_FILENAME, &shell_exit_status);
    switch (rc)
    {
        case SYSFUN_OK:
            /* do nothing */
            break;
        case SYSFUN_RESULT_ERROR:
            USBDISK_MOUNT_DEBUG_MSG("%s(%d): An error occurred when executing the script\r\n", __FUNCTION__, __LINE__);
            return FALSE;
            break;
        case SYSFUN_RESULT_CHILD_PROC_ERR:
            USBDISK_MOUNT_DEBUG_MSG("%s(%d): Child process abnormal termintaed.\r\n", __FUNCTION__, __LINE__);
            return FALSE;
            break;
        case SYSFUN_RESULT_SHELL_CMD_ERR:
            USBDISK_MOUNT_DEBUG_MSG("%s(%d): USB mount script returns error(exit code=%d)\r\n", __FUNCTION__, __LINE__, shell_exit_status);
            return FALSE;
            break;
        case SYSFUN_RESULT_INVALID_ARG:
            USBDISK_MOUNT_DEBUG_MSG("%s(%d): Invalid argument for SYSFUN_ExecuteSystemShellEx\r\n", __FUNCTION__, __LINE__);
            return FALSE;
            break;
        default:
            USBDISK_MOUNT_DEBUG_MSG("%s(%d): Unknown SYSFUN_ExecuteSystemShellEx error.(ret=%lu)\r\n", __FUNCTION__, __LINE__, (unsigned long)rc);
            return FALSE;
            break;
    }

    return TRUE;
#else /* #if (SYS_CPNT_FS_USE_SCRIPT_TO_MOUNT_USBDISK == TRUE) */
    struct stat file_stat;
    DIR *dirstream_p;
    struct dirent *file_p;
    char  *usb_storage_dir = "/proc/scsi/usb-storage";

    if(0 != access (usb_storage_dir, F_OK))
    {
        /*sda is not ready*/
        return FALSE;
    }

    dirstream_p = opendir(usb_storage_dir);
    if(NULL == dirstream_p)
    {
              /*bad /proc/scsi/usb-storage*/
        return FALSE;
    }

    file_p = readdir (dirstream_p);

    if( NULL == file_p)
    {
              /*sda1 is not ready*/
        return FALSE;
    }

    closedir(dirstream_p);

    if(stat("/mnt/usb/",&file_stat) < 0)
    {
        return FALSE;
    }

    if(0 == file_stat.st_size)
    {
        if(system("mount -t vfat /dev/sda1 /mnt/usb >/dev/null 2>&1")<0)
        {
            return FALSE;
        }
    }

    return TRUE;
#endif /* end of #if (SYS_CPNT_FS_USE_SCRIPT_TO_MOUNT_USBDISK == TRUE) */
}

static BOOL_T umount_usbdisk()
{
/*umount /mnt/usb >/dev/null 2>&1*/

    struct stat file_stat;
    if(stat("/mnt/usb/",&file_stat) < 0)
    {
        return FALSE;
    }


    if(0 == file_stat.st_size)
    {
           /*already umount*/
        return TRUE;
    }


    if(system("umount /mnt/usb >/dev/null 2>&1")<0)
    {
           return FALSE;
    }
    return TRUE;
}
/*gxz add,for mpc8347 usb,2009-04-09,end*/

/*------------------------------------------------------------------------
 * ROUTINE NAME - get_file_copy_user_info
 *------------------------------------------------------------------------
 * FUNCTION: Get file copy user information
 * INPUT   : user_info_p  -- user information entry
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T
get_file_copy_user_info(
    XFER_MGR_UserInfo_T *user_info_p)
{
    CLI_TASK_WorkingArea_T  *ctrl_P;
    L_INET_AddrIp_T         nexthop_addr;

    ctrl_P = CLI_TASK_GetMyWorkingArea();

    if (NULL == ctrl_P)
    {
        return FALSE;
    }

    switch (ctrl_P->CMenu.RemoteSessionType)
    {
        case CLI_TYPE_UART:
            user_info_p->session_type = VAL_trapVarSessionType_console;
            break;

        case CLI_TYPE_TELNET:
            user_info_p->session_type = VAL_trapVarSessionType_telnet;
            break;

        case CLI_TYPE_SSH:
            user_info_p->session_type = VAL_trapVarSessionType_ssh;
            break;

        default:
            return FALSE;
    }

    strncpy(user_info_p->user_name, ctrl_P->CMenu.UserName,
        sizeof(user_info_p->user_name)-1);
    user_info_p->user_name[sizeof(user_info_p->user_name)-1] = '\0';
    memcpy(&user_info_p->user_ip, &ctrl_P->CMenu.TelnetIPAddress,
        sizeof(user_info_p->user_ip));

    /* use user ip to get user mac
     */
    if (0 != ctrl_P->CMenu.TelnetIPAddress.addrlen)
    {
        if (NETCFG_TYPE_FAIL == NETCFG_PMGR_ROUTE_GetReversePathIpMac(
            &ctrl_P->CMenu.TelnetIPAddress, &nexthop_addr, user_info_p->user_mac))
        {
            return FALSE;
        }
    }

    return TRUE;
}/* End of get_file_copy_user_info */
