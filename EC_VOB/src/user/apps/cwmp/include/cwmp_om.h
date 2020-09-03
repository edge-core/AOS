/*-----------------------------------------------------------------------------
 * FILE NAME: CWMP_OM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Declares the APIs of CWMP OM.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/12/19     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef CWMP_OM_H
#define CWMP_OM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"
#include "cwmp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define CWMP_OM_IPCMSG_TYPE_SIZE sizeof(union CWMP_OM_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    CWMP_OM_IPC_GETURL,
    CWMP_OM_IPC_GETRUNNINGURL,
    CWMP_OM_IPC_GETUSERNAME,
    CWMP_OM_IPC_GETRUNNINGUSERNAME,
    CWMP_OM_IPC_GETPASSWORD,
    CWMP_OM_IPC_GETRUNNINGPASSWORD,
    CWMP_OM_IPC_GETPERIODICINFORMENABLE,
    CWMP_OM_IPC_GETRUNNINGPERIODICINFORMENABLE,
    CWMP_OM_IPC_GETPERIODICINFORMINTERVAL,
    CWMP_OM_IPC_GETRUNNINGPERIODICINFORMINTERVAL,
    CWMP_OM_IPC_GETCONNECTIONREQUESTUSERNAME,
    CWMP_OM_IPC_GETRUNNINGCONNECTIONREQUESTUSERNAME,
    CWMP_OM_IPC_GETCONNECTIONREQUESTPASSWORD,
    CWMP_OM_IPC_GETRUNNINGCONNECTIONREQUESTPASSWORD,
    CWMP_OM_IPC_GETCWMPCONFIGENTRY
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in CWMP_OM_IpcMsg_T.data
 */
#define CWMP_OM_GET_MSG_SIZE(field_name)                        \
            (CWMP_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((CWMP_OM_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */
typedef struct
{
    union CWMP_OM_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
		UI32_T ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        BOOL_T                  arg_bool;
        UI32_T                  arg_ui32;
        char                    arg_ar257[CWMP_TYPE_STR_LEN_256+1];
        CWMP_TYPE_ConfigEntry_T arg_config_entry;
    } data;
} CWMP_OM_IpcMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if 0
BOOL_T CWMP_OM_GetCwmpStatus(BOOL_T *status);
BOOL_T CWMP_OM_SetCwmpStatus(BOOL_T status);
#endif
BOOL_T CWMP_OM_GetUrl(char *url);
UI32_T CWMP_OM_GetRunningUrl(char *url);
BOOL_T CWMP_OM_SetUrl(char *url);

BOOL_T CWMP_OM_GetUsername(char *username);
UI32_T CWMP_OM_GetRunningUsername(char *username);
BOOL_T CWMP_OM_SetUsername(char *username);

BOOL_T CWMP_OM_GetPassword(char *password);
UI32_T CWMP_OM_GetRunningPassword(char *password);
BOOL_T CWMP_OM_SetPassword(char *password);

BOOL_T CWMP_OM_GetPeriodicInformEnable(BOOL_T *status);
UI32_T CWMP_OM_GetRunningPeriodicInformEnable(BOOL_T *status);
BOOL_T CWMP_OM_SetPeriodicInformEnable(BOOL_T status);

BOOL_T CWMP_OM_GetPeriodicInformInterval(UI32_T *interval);
UI32_T CWMP_OM_GetRunningPeriodicInformInterval(UI32_T *interval);
BOOL_T CWMP_OM_SetPeriodicInformInterval(UI32_T interval);

BOOL_T CWMP_OM_GetPeriodicInformTime(char *time_reference);

BOOL_T CWMP_OM_GetParameterKey(char* param_key);

BOOL_T CWMP_OM_GetConnectionRequestUrl(char* cr_url);
BOOL_T CWMP_OM_SetConnectionRequestUrl(char* cr_url);

BOOL_T CWMP_OM_GetConnectionRequestUsername(char* cr_username);
UI32_T CWMP_OM_GetRunningConnectionRequestUsername(char* cr_username);
BOOL_T CWMP_OM_SetConnectionRequestUsername(char* cr_username);

BOOL_T CWMP_OM_GetConnectionRequestPassword(char* cr_password);
UI32_T CWMP_OM_GetRunningConnectionRequestPassword(char* cr_password);
BOOL_T CWMP_OM_SetConnectionRequestPassword(char* cr_password);

BOOL_T CWMP_OM_GetCwmpConfigEntry(CWMP_TYPE_ConfigEntry_T *config_entry_p);


#endif /* #ifndef CWMP_OM_H */
