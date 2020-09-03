/*-------------------------------------------------------------------------
 * Module Name  :   dhcp_backdoor.h
 *-------------------------------------------------------------------------
 * Purpose      :   This file supports a backdoor for DHCP
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    06/10/2002 -  Penny Chang, created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-------------------------------------------------------------------------
 */

#ifndef _DHCP_BACKDOOR_H_
#define _DHCP_BACKDOOR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "backdoor_mgr.h"
#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_mgr.h"
#endif
/* NAMING CONST DECLARATIONS
 */
typedef enum DHCP_BD_DebugFlag_E
{
#if(SYS_CPNT_DEBUG == TRUE)
    DHCP_BD_FLAG_NONE           = DEBUG_TYPE_DHCP_NONE,
    DHCP_BD_FLAG_CONFIG         = DEBUG_TYPE_DHCP_CONFIG,
    DHCP_BD_FLAG_EVENT          = DEBUG_TYPE_DHCP_EVENT,
    DHCP_BD_FLAG_PACKET         = DEBUG_TYPE_DHCP_PACKET,
    DHCP_BD_FLAG_CLIENT         = DEBUG_TYPE_DHCP_CLIENT,
    DHCP_BD_FLAG_RELAY          = DEBUG_TYPE_DHCP_RELAY,
    DHCP_BD_FLAG_SERVER         = DEBUG_TYPE_DHCP_SERVER,
    DHCP_BD_FLAG_DATABASE       = DEBUG_TYPE_DHCP_DATABASE,
    DHCP_BD_FLAG_ALL            = DEBUG_TYPE_DHCP_ALL,
#else
    DHCP_BD_FLAG_NONE           = 0x00000000L,
    DHCP_BD_FLAG_CONFIG         = 0x00000100L,
    DHCP_BD_FLAG_EVENT          = 0x00000200L,
    DHCP_BD_FLAG_PACKET         = 0x00000400L,
    DHCP_BD_FLAG_CLIENT         = 0x00000001L,
    DHCP_BD_FLAG_RELAY          = 0x00000002L,
    DHCP_BD_FLAG_SERVER         = 0x00000004L,
    DHCP_BD_FLAG_DATABASE       = 0x00000800L,
    DHCP_BD_FLAG_ALL            = 0xFFFFFF00L,
#endif
}DHCP_BD_DebugFlag_T;

#define DHCP_BD_STR_CONFIG     "CONFIG"
#define DHCP_BD_STR_EVENT      "EVENT"
#define DHCP_BD_STR_PACKET     "PACKET"
#define DHCP_BD_STR_CLIENT     "CLIENT"
#define DHCP_BD_STR_RELAY      "RELAY"
#define DHCP_BD_STR_SERVER     "SERVER"
#define DHCP_BD_STR_DATABASE   "DATABASE"

/* MACRO FUNCTION DECLARATIONS
 */
#define DHCP_BD_MSG(__str, __arg...)  BACKDOOR_MGR_Printf(__str, ##__arg);
#define DHCP_BD_GetLine(buf, size) ({ BACKDOOR_MGR_RequestKeyIn((buf), (size)); strlen((char *)buf); })

#if (SYS_CPNT_DEBUG == TRUE)
#define DHCP_BD(__flag, format,...)         \
    do \
    {           \
        DEBUG_MGR_Printf(DEBUG_TYPE_DHCP, \
                         DEBUG_TYPE_MATCH_ANY, \
                         (DHCP_BD_FLAG_##__flag), \
                         0, \
                         "DHCP Debug[%s]: "format"\r\n",DHCP_BD_STR_##__flag,##__VA_ARGS__); \
        if (DHCP_BACKDOOR_GetDebugFlag((DHCP_BD_FLAG_##__flag)))               \
        {                                           \
            DHCP_BD_MSG("%s[%d], ", __FUNCTION__, __LINE__); \
            DHCP_BD_MSG(format,##__VA_ARGS__); \
            DHCP_BD_MSG("\r\n"); \
        }                                           \
    }while(0)
#else
#define DHCP_BD(__flag, format,...)         \
    do \
    {           \
        if (DHCP_BACKDOOR_GetDebugFlag((DHCP_BD_FLAG_##__flag)))               \
        {                                           \
            DHCP_BD_MSG("%s[%d], ", __FUNCTION__, __LINE__); \
            DHCP_BD_MSG(format,##__VA_ARGS__); \
            DHCP_BD_MSG("\r\n"); \
        }                                           \
    }while(0)
#endif


/* DATA TYPE DECLARATIONS
 */
enum
{
    /* Main menu index */
    DHCP_BACKDOOR_MAIN_MENU_CONFIG_INDEX = 1,
    DHCP_BACKDOOR_MAIN_MENU_DATABASE_INDEX,
    DHCP_BACKDOOR_MAIN_MENU_DEBUG_INDEX,
    DHCP_BACKDOOR_MAIN_MENU_END_INDEX,
    
    
    /* Config menu index */
    DHCP_BACKDOOR_CONFIG_MENU_TRAP_PKT_INDEX = 1,
    DHCP_BACKDOOR_CONFIG_MENU_SEND_INFORM_PKT_INDEX,
    DHCP_BACKDOOR_CONFIG_MENU_END_INDEX,
    
    /* Database menu index */
    DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_WA_INTF_INFO = 1,
    DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_OM_LCB_INFO,
    DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_OM_UC_DATA,
    DHCP_BACKDOOR_DATABASE_MENU_DISPLAY_OM_INTF_INFO,
    DHCP_BACKDOOR_DATABASE_MENU_END_INDEX,
    
    /* Debug menu index */
    DHCP_BACKDOOR_DEBUG_MENU_SET_FLAG_INDEX = 1,
    DHCP_BACKDOOR_DEBUG_MENU_END_INDEX,
    
};


typedef struct DHCP_BACKDOOR_CommandStruct_S
{
    UI8_T  cmd_idx;
    char   *cmd_description;
    void (*cmd_function)(void);
} DHCP_BACKDOOR_CommandStruct_T;

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DHCP_BACKDOOR_Init
 * ------------------------------------------------------------------------
 * FUNCTION : This function ititiates the backdoor function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DHCP_BACKDOOR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_BACKDOOR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DHCP_BACKDOOR_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_BACKDOOR_GetDebugFlag
 *------------------------------------------------------------------------------
 * PURPOSE : get the debug flag is on or off
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - this debug flag is on
 *           FALSE - this debug flag is off
 * NOTE    :
 *------------------------------------------------------------------------------
 */
BOOL_T  DHCP_BACKDOOR_GetDebugFlag(UI32_T flag);

#endif /* _DHCP_BACKDOOR_H_ */
