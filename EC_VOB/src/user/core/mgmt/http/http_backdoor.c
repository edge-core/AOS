/* MODULE NAME: http_backdoor.c
 * PURPOSE:
 *	Implementations for the HTTP backdoor

 * NOTES:
 *	None.
 *
 * HISTORY:
 *    mm/dd/yy (A.D.)
 *    07/13/15    -- Emma, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "http_loc.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MAXLINE 255

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_CPNT_HTTPS == TRUE)
static void HTTP_BACKDOOR_DisplayHttpsCertificate();
#endif /* SYS_CPNT_HTTPS */

static void HTTP_BACKDOOR_SetConfigFilePath();
static void HTTP_BACKDOOR_DisplayConfigFilePath();
static void HTTP_BACKDOOR_ReloadConfig();
static void HTTP_BACKDOOR_DisplayConfig();
static void HTTP_BACKDOOR_RestartServer();


/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  HTTP_BACKDOOR_Main
 * PURPOSE:
 *          Display back door available function and accept user seletion.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_BACKDOOR_Main()
{
    UI8_T   keyin;
#if( SYS_CPNT_HTTPS == TRUE )
    HTTP_CertificateInfo_T  cert_info;
    BOOL_T  ret;
#endif

    while(1)
    {
#if (SYS_CPNT_HTTPS == TRUE)
        BACKDOOR_MGR_Printf("\r\n1. Display HTTPS certificate.");
#endif /* SYS_CPNT_HTTPS */

        // TODO: could reload config file via linux shell
        // and could also set config via linux shell
        BACKDOOR_MGR_Printf("\r\n2. Set config file path.");
        BACKDOOR_MGR_Printf("\r\n3. Display config file path.");
        BACKDOOR_MGR_Printf("\r\n4. Reload config.");
        BACKDOOR_MGR_Printf("\r\n5. Display config.");
        BACKDOOR_MGR_Printf("\r\n6. Restart server.");
        BACKDOOR_MGR_Printf("\r\n0. Exit.\r\n");
        BACKDOOR_MGR_Printf("\r\nEnter your choice: ");

        keyin = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("\r\n");

        switch(keyin)
        {
            case '0':
                return;

#if (SYS_CPNT_HTTPS == TRUE)
            case '1':
                HTTP_BACKDOOR_DisplayHttpsCertificate();
                break;
#endif /* SYS_CPNT_HTTPS */

            case '2':
                HTTP_BACKDOOR_SetConfigFilePath();
                break;

            case '3':
                HTTP_BACKDOOR_DisplayConfigFilePath();
                break;

            case '4':
                HTTP_BACKDOOR_ReloadConfig();
                break;

            case '5':
                HTTP_BACKDOOR_DisplayConfig();
                break;

            case '6':
                HTTP_BACKDOOR_RestartServer();
                break;

            default:
                continue;
      }
      BACKDOOR_MGR_Printf("\r\n\r\n\r\n\r\n");
      BACKDOOR_MGR_Printf("---------------------------\r\n");
   }

}

/* LOCAL SUBPROGRAM BODIES
 */
#if (SYS_CPNT_HTTPS == TRUE)
static void HTTP_BACKDOOR_DisplayHttpsCertificate()
{
    HTTP_CertificateInfo_T  cert_info;
    BOOL_T  ret;

    ret = HTTP_MGR_Get_Certificate_Info(&cert_info);
    if( ret == TRUE )
    {
        BACKDOOR_MGR_Printf("subject = %s\n",cert_info.subject);
        BACKDOOR_MGR_Printf("issuer = %s\n",cert_info.issuer);
        BACKDOOR_MGR_Printf("valid_begin = %s\n",cert_info.valid_begin);
        BACKDOOR_MGR_Printf("valid_end = %s\n",cert_info.valid_end);
        BACKDOOR_MGR_Printf("sha1_fingerprint = %s\n",cert_info.sha1_fingerprint);
        BACKDOOR_MGR_Printf("md5_fingerprint = %s\n",cert_info.md5_fingerprint);
    }
    else
    {
        BACKDOOR_MGR_Printf("fail\n");
    }
}
#endif /* SYS_CPNT_HTTPS */

static void HTTP_BACKDOOR_SetConfigFilePath()
{
    char file_path[HTTP_CONFIG_FILE_PATH_MAX_LEN + 1];

    BACKDOOR_MGR_Printf("\r\nPlease input the config file path: ");
    BACKDOOR_MGR_RequestKeyIn((char *)file_path, sizeof(file_path) - 1);

    if (FALSE == HTTP_OM_SetConfigFilePath(file_path))
    {
        BACKDOOR_MGR_Printf("\r\nFailed to set config file path.");
    }

    if (FALSE == HTTP_MGR_LoadConfig(file_path))
    {
        BACKDOOR_MGR_Printf("\r\nFailed to load config file.");
    }

    BACKDOOR_MGR_Printf("\r\nSucceed.");
}

static void HTTP_BACKDOOR_DisplayConfigFilePath()
{
    char file_path[HTTP_CONFIG_FILE_PATH_MAX_LEN + 1];

    if (FALSE == HTTP_OM_GetConfigFilePath(file_path, sizeof(file_path)))
    {
        BACKDOOR_MGR_Printf("\r\nFailed to get config file path.");
        return;
    }

    BACKDOOR_MGR_Printf("\r\nThe config file path is %s", file_path);
}

static void HTTP_BACKDOOR_ReloadConfig()
{
    char file_path[HTTP_CONFIG_FILE_PATH_MAX_LEN + 1];

    if (FALSE == HTTP_OM_GetConfigFilePath(file_path, sizeof(file_path)))
    {
        BACKDOOR_MGR_Printf("\r\nFailed to get config file path.");
        return;
    }

    if (FALSE == HTTP_MGR_LoadConfig(file_path))
    {
        BACKDOOR_MGR_Printf("\r\nFailed to load config file.");
        return;
    }

    BACKDOOR_MGR_Printf("\r\nSucceed.");
}

static void HTTP_BACKDOOR_DisplayConfig()
{
    char file_path[HTTP_CONFIG_FILE_PATH_MAX_LEN + 1];
    char root_dir[HTTP_CONFIG_ROOT_DIR_MAX_LEN + 1];

    if (FALSE == HTTP_OM_GetConfigFilePath(file_path, sizeof(file_path)))
    {
        BACKDOOR_MGR_Printf("\r\nFailed to get config file path.");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nFile Path: %s", file_path);
    }

    if (FALSE == HTTP_OM_Get_Root_Dir(root_dir, sizeof(root_dir)))
    {
        BACKDOOR_MGR_Printf("\r\nFailed to get root dir.");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nRoot Dir: %s", root_dir);
    }

    BACKDOOR_MGR_Printf("\r\nSucceed.");
}

static void HTTP_BACKDOOR_RestartServer()
{
    HTTP_TASK_Port_Changed();
#if (SYS_CPNT_HTTPS == TRUE)
    HTTP_TASK_Secure_Port_Changed();
#endif
}

