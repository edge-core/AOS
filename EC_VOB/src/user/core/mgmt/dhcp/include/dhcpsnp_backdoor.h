/* Module Name: DHCPSNP_BACKDOOR.H
 * Purpose:
 *      DHCPSNP_BACKDOOR provides debug control and directly access related information.
 *
 * Notes:
 *      
 *
 * History:
 *          Date    --  Modifier,  Reason
 *    2012/ 2/ 3    -- jimi_chen, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */
#ifndef _DHCPSNP_BACKDOOR_H
#define _DHCPSNP_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS 
 */
#include "sys_cpnt.h"
#include "backdoor_mgr.h"
#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_mgr.h"
#endif

/* NAMING CONST DECLARATIONS 
 */
typedef enum DHCPSNP_BD_DebugFlag_E
{
#if(SYS_CPNT_DEBUG == TRUE)
    DHCPSNP_BD_FLAG_NONE           = DEBUG_TYPE_DHCPSNP_NONE,
    DHCPSNP_BD_FLAG_CONFIG         = DEBUG_TYPE_DHCPSNP_CONFIG,
    DHCPSNP_BD_FLAG_EVENT          = DEBUG_TYPE_DHCPSNP_EVENT,
    DHCPSNP_BD_FLAG_PACKET         = DEBUG_TYPE_DHCPSNP_PACKET,
    DHCPSNP_BD_FLAG_DATABASE       = DEBUG_TYPE_DHCPSNP_DATABASE,
    DHCPSNP_BD_FLAG_ALL            = DEBUG_TYPE_DHCPSNP_ALL,
#else
    DHCPSNP_BD_FLAG_NONE           = 0x00000000L,
    DHCPSNP_BD_FLAG_CONFIG         = 0x00000100L,
    DHCPSNP_BD_FLAG_EVENT          = 0x00000200L,
    DHCPSNP_BD_FLAG_PACKET         = 0x00000400L,
    DHCPSNP_BD_FLAG_DATABASE       = 0x00000800L,
    DHCPSNP_BD_FLAG_ALL            = 0xFFFFFF00L,
#endif
}DHCPSNP_BD_DebugFlag_T;

#define DHCPSNP_BD_STR_CONFIG     "CONFIG"
#define DHCPSNP_BD_STR_EVENT      "EVENT"
#define DHCPSNP_BD_STR_PACKET     "PACKET"
#define DHCPSNP_BD_STR_DATABASE   "DATABASE"

/* EXTERN VARIABLE DECLARATIONS 
 */

/* MACRO FUNCTION DECLARATIONS 
 */
#define DHCPSNP_BD_MSG(__str, __arg...)  BACKDOOR_MGR_Printf(__str, ##__arg);
#define DHCPSNP_BD_GetLine(buf, size) ({ BACKDOOR_MGR_RequestKeyIn((buf), (size)); strlen((char *)buf); })

#if (SYS_CPNT_DEBUG == TRUE)
#define DHCPSNP_BD(__flag, format,...)         \
do \
{           \
      DEBUG_MGR_Printf(DEBUG_TYPE_DHCPSNP, \
                       DEBUG_TYPE_MATCH_ANY, \
                       (DHCPSNP_BD_FLAG_##__flag), \
                       0, \
                       "DHCPSNP Debug[%s]: "format"\r\n",DHCPSNP_BD_STR_##__flag,##__VA_ARGS__); \
    if (DHCPSNP_BACKDOOR_GetDebugFlag((DHCPSNP_BD_FLAG_##__flag)))               \
    {                                           \
        DHCPSNP_BD_MSG("%s[%d], ", __FUNCTION__, __LINE__); \
        DHCPSNP_BD_MSG(format,##__VA_ARGS__); \
        DHCPSNP_BD_MSG("\r\n"); \
    }                                           \
}while(0)
#else
#define DHCPSNP_BD(__flag, format,...)         \
do \
{           \
    if (DHCPSNP_BACKDOOR_GetDebugFlag((DHCPSNP_BD_FLAG_##__flag)))               \
    {                                           \
        DHCPSNP_BD_MSG("%s[%d], ", __FUNCTION__, __LINE__); \
        DHCPSNP_BD_MSG(format,##__VA_ARGS__); \
        DHCPSNP_BD_MSG("\r\n"); \
    }                                           \
}while(0)
#endif


/* DATA TYPE DECLARATIONS 
 */

enum 
{   
    /* Main menu index */
    DHCPSNP_BACKDOOR_MAIN_MENU_CONFIG_INDEX = 1,
    DHCPSNP_BACKDOOR_MAIN_MENU_DATABASE_INDEX,
    DHCPSNP_BACKDOOR_MAIN_MENU_DEBUG_INDEX,
    DHCPSNP_BACKDOOR_MAIN_MENU_END_INDEX,

    /* Config menu index */
    DHCPSNP_BACKDOOR_CONFIG_MENU_ADD_BINDING_INDEX = 1,
    DHCPSNP_BACKDOOR_CONFIG_MENU_DEL_BINDING_INDEX,
    DHCPSNP_BACKDOOR_CONFIG_MENU_ADD_MULTI_BINDING_INDEX,
    DHCPSNP_BACKDOOR_CONFIG_MENU_CLEAR_STATISTIC_CNT_INDEX,
    DHCPSNP_BACKDOOR_CONFIG_MENU_SET_REMOTE_ID_MODE_INDEX,
    DHCPSNP_BACKDOOR_CONFIG_MENU_SET_CIRCUIT_ID_MODE_INDEX,
    DHCPSNP_BACKDOOR_CONFIG_MENU_END_INDEX,

    /* Database menu index */
    DHCPSNP_BACKDOOR_DATABASE_MENU_DISPLAY_PORT_INFO = 1,
    DHCPSNP_BACKDOOR_DATABASE_MENU_DISPLAY_COUNTER,
    DHCPSNP_BACKDOOR_DATABASE_MENU_DISPLAY_BINDING_INFO,
    DHCPSNP_BACKDOOR_DATABASE_MENU_DISPLAY_SYSTEM_RATE,
    DHCPSNP_BACKDOOR_DATABASE_MENU_DISPLAY_GLOBAL_INFO,
    DHCPSNP_BACKDOOR_DATABASE_MENU_END_INDEX,


};


typedef struct DHCPSNP_BACKDOOR_CommandStruct_S
{
    UI8_T  cmd_idx;
    char   *cmd_description;
    void   (*cmd_function)(void);
} DHCPSNP_BACKDOOR_CommandStruct_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS 
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_BACKDOOR_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------------
 */ 
void DHCPSNP_BACKDOOR_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_BACKDOOR_GetDebugFlag
 *------------------------------------------------------------------------------
 * PURPOSE : get the debug flag is on or off
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - this debug flag is on
 *           FALSE - this debug flag is off
 * NOTE    :
 *------------------------------------------------------------------------------
 */
BOOL_T  DHCPSNP_BACKDOOR_GetDebugFlag(UI32_T flag);

    
#endif
