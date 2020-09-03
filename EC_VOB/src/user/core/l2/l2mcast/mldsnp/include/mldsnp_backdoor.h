/* MODULE NAME: mldsnp_engine.H
* PURPOSE:
*   {1. What is covered in this file - function and scope}
*   {2. Related documents or hardware information}
* NOTES:
*   {Something must be known or noticed}
*   {1. How to use these functions - Give an example}
*   {2. Sequence of messages if applicable}
*   {3. Any design limitation}
*   {4. Any performance limitation}
*   {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2007
*/


#ifndef _MLDSNP_BACKDOOR_H
#define _MLDSNP_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
*/
#include "sys_type.h"
#include "l_inet.h"
#include "backdoor_mgr.h"
#if(SYS_CPNT_DEBUG == TRUE)
#include "debug_type.h"
#include "debug_mgr.h"
#endif
/* NAMING CONSTANT DECLARATIONS
*/

typedef enum MLDSNP_BD_DebugFlag_E
{
#if(SYS_CPNT_DEBUG == TRUE)
    MLDSNP_BD_FLAG_NONE           = DEBUG_TYPE_MLDSNP_NONE,
    MLDSNP_BD_FLAG_RX             = DEBUG_TYPE_MLDSNP_DECODE/*BIT_1*/, /*rx packet flow*/
    MLDSNP_BD_FLAG_TX             = DEBUG_TYPE_MLDSNP_ENCODE/*BIT_2*/, /*tx packet flow*/
#else
    MLDSNP_BD_FLAG_NONE           = 0,
    MLDSNP_BD_FLAG_RX             = BIT_1, /*rx packet flow*/
    MLDSNP_BD_FLAG_TX             = BIT_2, /*tx packet flow*/
#endif
    MLDSNP_BD_FLAG_TIMER          = BIT_3, /*timer envent*/
    MLDSNP_BD_FLAG_UI             = BIT_4, /*UI call which function*/
    MLDSNP_BD_FLAG_IPC            = BIT_5, /*IPC call which function*/
    MLDSNP_BD_FLAG_TRACE	      = BIT_6, /*trace the code flow*/
    MLDSNP_BD_FLAG_CALLBACK       = BIT_7, /*other csc notify*/
    MLDSNP_BD_FLAG_ERR            = BIT_8, /*it shall success but fail*/
    MLDSNP_BD_FLAG_ALL            = 0xFFFFFFFFUL,
}MLDSNP_BD_DebugFlag_T;

/* MACRO FUNCTION DECLARATIONS
*/
#define MLDSNP_BD_MSG(__str, ...)  BACKDOOR_MGR_Printf(__str, ##__VA_ARGS__);

#if (SYS_CPNT_DEBUG == TRUE)
#define MLDSNP_BD(__flag, ...)         \
do \
{                                               \
	if(( (MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_RX \
       ||(MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_TX))\
	{ \
      DEBUG_MGR_Printf(DEBUG_TYPE_MLDSNP, \
                       DEBUG_TYPE_MATCH_ANY, \
                       (MLDSNP_BD_FLAG_##__flag), \
                       0, \
                       "MLDsnp Debug: %s \r\n",## __VA_ARGS__); \
    } if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag)))               \
    {                                           \
        MLDSNP_BD_MSG("(%d):%s, ", __LINE__, __FUNCTION__); \
        MLDSNP_BD_MSG(__VA_ARGS__); \
        MLDSNP_BD_MSG("\r\n"); \
    }                                           \
}while(0)
#define MLDSNP_BD_ARG(__flag, ...)         \
do \
{                                               \
    if(( (MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_RX \
       ||(MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_TX))\
    { \
      DEBUG_MGR_Printf(DEBUG_TYPE_MLDSNP, \
                       DEBUG_TYPE_MATCH_ANY, \
                       (MLDSNP_BD_FLAG_##__flag), \
                       0, \
                       "MLDsnp Debug:"); \
      DEBUG_MGR_Printf(DEBUG_TYPE_MLDSNP, \
                       DEBUG_TYPE_MATCH_ANY, \
                       (MLDSNP_BD_FLAG_##__flag), \
                       0, \
                       ## __VA_ARGS__); \
    } if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag)))               \
    {                                           \
        MLDSNP_BD_MSG("(%d):%s, ", __LINE__, __FUNCTION__); \
        MLDSNP_BD_MSG(__VA_ARGS__); \
        MLDSNP_BD_MSG("\r\n"); \
    }                                           \
}while(0)

#define MLDSNP_BD_SHOW_SRC(__flag, __MSG, __src_ip, __num_of_src)  \
do {   \
    UI16_T i=0; \
    UI8_T *src_p=(__src_ip);  \
    char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1] ={0};        \
    if(( (MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_RX \
       ||(MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_TX))\
	{ \
        if(NULL!=src_p) \
		{  \
            for(;i<__num_of_src;src_p+=MLDSNP_TYPE_IPV6_SRC_IP_LEN, i++) \
			{  \
                L_INET_Ntop(L_INET_AF_INET6, (void *)src_p, ipv6_addr_str, sizeof(ipv6_addr_str)); \
                DEBUG_MGR_Printf(DEBUG_TYPE_MLDSNP, \
                                 DEBUG_TYPE_MATCH_ANY, \
                                 (MLDSNP_BD_FLAG_##__flag), \
                                 0, \
                                 "MLDsnp Debug: %s, (%d):src IPv6 %s\r\n",__MSG,  i, ipv6_addr_str); \
            }\
        }\
    }if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag))){               \
        if(NULL!=src_p){  \
            MLDSNP_BD_MSG(__MSG);  \
            for(;i<__num_of_src;src_p+=MLDSNP_TYPE_IPV6_SRC_IP_LEN, i++)  \
            { \
                L_INET_Ntop(L_INET_AF_INET6, (void *)src_p, ipv6_addr_str, sizeof(ipv6_addr_str)); \
                MLDSNP_BD_MSG("(%d):src ip %s\r\n", i, ipv6_addr_str); \
            } \
        } \
    } \
}while(0)

#define MLDSNP_BD_SHOW_GROUP(__flag, __MSG, __group_ip, ...)  \
do{   \
    char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1] ={0};        \
    if(( (MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_RX \
       ||(MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_TX)) \
{   \
        L_INET_Ntop(L_INET_AF_INET6, (void *)__group_ip, ipv6_addr_str, sizeof(ipv6_addr_str)); \
        DEBUG_MGR_Printf(DEBUG_TYPE_MLDSNP, \
                         DEBUG_TYPE_MATCH_ANY, \
                         (MLDSNP_BD_FLAG_##__flag), \
                         0, \
                         "MLDsnp Debug:"__MSG", group \r\n", ##__VA_ARGS__, ipv6_addr_str ); \
    }if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag)))               \
    { \
        /*if(NULL != __group_ip)*/ \
            L_INET_Ntop(L_INET_AF_INET6, (void *)__group_ip, ipv6_addr_str, sizeof(ipv6_addr_str)); \
        MLDSNP_BD_MSG("(%d):%s, "__MSG ", group ip %s\r\n",  __LINE__, __FUNCTION__, ##__VA_ARGS__, ipv6_addr_str ); \
    } \
}while(0)

#define MLDSNP_BD_SHOW_GROUP_SRC(__flag, __MSG, __group_ip,  __src_ip, __num_of_src, ...)  \
do{   \
        UI16_T i=0; \
        UI8_T *src_p=(__src_ip);  \
        char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1] ={0};        \
    if(( (MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_RX \
       ||(MLDSNP_BD_FLAG_##__flag) == MLDSNP_BD_FLAG_TX)) \
    {\
        if(NULL != __group_ip) \
            L_INET_Ntop(L_INET_AF_INET6, (void *)__group_ip, ipv6_addr_str, sizeof(ipv6_addr_str)); \
        DEBUG_MGR_Printf(DEBUG_TYPE_MLDSNP, \
                         DEBUG_TYPE_MATCH_ANY, \
                         (MLDSNP_BD_FLAG_##__flag), \
                         0, \
                         "MLDsnp Debug:"__MSG", group %s\r\n", ##__VA_ARGS__, ipv6_addr_str ); \
        if(NULL!=src_p)  \
        { \
            for(;i<__num_of_src;src_p+=MLDSNP_TYPE_IPV6_SRC_IP_LEN, i++)  \
            { \
                L_INET_Ntop(L_INET_AF_INET6, (void *)src_p, ipv6_addr_str, sizeof(ipv6_addr_str)); \
                DEBUG_MGR_Printf(DEBUG_TYPE_MLDSNP, \
                                 DEBUG_TYPE_MATCH_ANY, \
                                 (MLDSNP_BD_FLAG_##__flag), \
                                 0, \
                                 "             (%d):src %s\r\n", i, ipv6_addr_str ); \
            } \
        }\
    }if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag)))               \
    { \
        if(NULL != __group_ip) \
            L_INET_Ntop(L_INET_AF_INET6, (void *)__group_ip, ipv6_addr_str, sizeof(ipv6_addr_str)); \
        MLDSNP_BD_MSG("(%d):%s, "__MSG ", group ip %s\r\n", __LINE__, __FUNCTION__, ##__VA_ARGS__, ipv6_addr_str ); \
        if(NULL!=src_p)  \
        { \
            for(;i<__num_of_src;src_p+=MLDSNP_TYPE_IPV6_SRC_IP_LEN, i++)  \
            { \
                L_INET_Ntop(L_INET_AF_INET6, (void *)src_p, ipv6_addr_str, sizeof(ipv6_addr_str)); \
                MLDSNP_BD_MSG("(%d):src ip %s\r\n", i, ipv6_addr_str); \
            } \
        } \
    } \
}while(0)
#else
#define MLDSNP_BD(__flag, ...)         \
do \
{                                               \
    if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag)))               \
    {                                           \
        MLDSNP_BD_MSG("(%d):%s, ", __LINE__, __FUNCTION__); \
        MLDSNP_BD_MSG(__VA_ARGS__); \
        MLDSNP_BD_MSG("\r\n"); \
    }                                           \
}while(0)

#define MLDSNP_BD_ARG(__flag, ...)         \
do \
{                                               \
    if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag)))               \
    {                                           \
        MLDSNP_BD_MSG("(%d):%s, ", __LINE__, __FUNCTION__); \
        MLDSNP_BD_MSG(__VA_ARGS__); \
        MLDSNP_BD_MSG("\r\n"); \
    }                                           \
}while(0)

#define MLDSNP_BD_SHOW_SRC(__flag, __MSG, __src_ip, __num_of_src)  \
do {   \
    if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag))){               \
        UI16_T i=0; \
        UI8_T *src_p=(__src_ip);  \
        char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1] ={0};        \
        if(NULL!=src_p){  \
            MLDSNP_BD_MSG(__MSG);  \
            for(;i<__num_of_src;src_p+=MLDSNP_TYPE_IPV6_SRC_IP_LEN, i++)  \
            { \
                L_INET_Ntop(L_INET_AF_INET6, (void *)src_p, ipv6_addr_str, sizeof(ipv6_addr_str)); \
                MLDSNP_BD_MSG("(%d):src ip %s\r\n", i, ipv6_addr_str); \
            } \
        } \
    } \
}while(0)

#define MLDSNP_BD_SHOW_GROUP(__flag, __MSG, __group_ip, ...)  \
do{   \
    if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag)))               \
    {   \
        char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1] ={0};        \
        /*if(NULL != __group_ip)*/ \
            L_INET_Ntop(L_INET_AF_INET6, (void *)__group_ip, ipv6_addr_str, sizeof(ipv6_addr_str)); \
        MLDSNP_BD_MSG("(%d):%s, "__MSG ", group ip %s\r\n",  __LINE__, __FUNCTION__, ##__VA_ARGS__, ipv6_addr_str ); \
    } \
}while(0)

#define MLDSNP_BD_SHOW_GROUP_SRC(__flag, __MSG, __group_ip,  __src_ip, __num_of_src, ...)  \
do{   \
    if (MLDSNP_BACKDOOR_GetDebug((MLDSNP_BD_FLAG_##__flag)))               \
    { \
        UI16_T i=0; \
        UI8_T *src_p=(__src_ip);  \
        char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1] ={0};        \
        if(NULL != __group_ip) \
            L_INET_Ntop(L_INET_AF_INET6, (void *)__group_ip, ipv6_addr_str, sizeof(ipv6_addr_str)); \
        MLDSNP_BD_MSG("(%d):%s, "__MSG ", group ip %s\r\n", __LINE__, __FUNCTION__, ##__VA_ARGS__, ipv6_addr_str ); \
        if(NULL!=src_p)  \
        { \
            for(;i<__num_of_src;src_p+=MLDSNP_TYPE_IPV6_SRC_IP_LEN, i++)  \
            { \
                L_INET_Ntop(L_INET_AF_INET6, (void *)src_p, ipv6_addr_str, sizeof(ipv6_addr_str)); \
                MLDSNP_BD_MSG("(%d):src ip %s\r\n", i, ipv6_addr_str); \
            } \
        } \
    } \
}while(0)
#endif

#define MLDSNP_BD_SHOW_SRC_MSG(__src_ip, __num_of_src)  \
{   \
    UI16_T i=0; \
    UI8_T *src_p=(__src_ip);  \
    char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1] ={0};        \
    if(NULL!=src_p)  \
        for(;i<__num_of_src;src_p+=MLDSNP_TYPE_IPV6_SRC_IP_LEN, i++)  \
        { \
                L_INET_Ntop(L_INET_AF_INET6, (void *)src_p, ipv6_addr_str, sizeof(ipv6_addr_str)); \
                MLDSNP_BD_MSG("(%d):src ip %s\r\n", i, ipv6_addr_str); \
        } \
}

#define MLDSNP_BD_SHOW_GROUP_MSG(__group_ip)  \
{   \
        char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1] ={0};        \
        if(NULL != __group_ip) \
            L_INET_Ntop(L_INET_AF_INET6, (void *)__group_ip, ipv6_addr_str, sizeof(ipv6_addr_str)); \
        MLDSNP_BD_MSG("group ip %s\r\n",ipv6_addr_str); \
}

/* DATA TYPE DECLARATIONS

*/
typedef BOOL_T (*mldsnp_bd_func_t)(void *);
typedef struct FuncName_S
{
    mldsnp_bd_func_t func_callback_p;
    char *func_name_p;
}FuncName_T;

/* LOCAL SUBPROGRAM BODIES
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_MainMenu
 *-------------------------------------------------------------------------
 * PURPOSE : print the main menu of the mldsnp debug backdoor
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_MainMenu();
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_MainMenu
 *-------------------------------------------------------------------------
 * PURPOSE : print the packet content
 * INPUT   : flag - debug flag
 *               *pdu_p - the packer content
 *               len - the pdu len to print
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_PrintPdu(
                        MLDSNP_BD_DebugFlag_T flag,
                        UI8_T *pdu_p,
                        UI16_T len);
/*------------------------------------------------------------------------------
* Function : MLDSNP_BACKDOOR_ShowTimerList
*------------------------------------------------------------------------------
* Purpose: This function is for debug to show all current timer content
* INPUT  : *timer_p - the new timer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is for backdoor
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_BACKDOOR_ShowTimerList();
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_RestAll
 *-------------------------------------------------------------------------
 * PURPOSE : reset all debug flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_RestAll();
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE :  set the debug flag
 * INPUT   : flag - debug enum define
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_SetDebugFlag(
                            UI32_T flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_GetDebug
 *-------------------------------------------------------------------------
 * PURPOSE : get the debug flag is on or off
 * INPUT   : flag - debug enum define
 * OUTPUT  : None
 * RETURN  : TRUE  - this debug flag is on
 *           FALSE - this debug flag is off
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T  MLDSNP_BACKDOOR_GetDebug(
                                UI32_T flag);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_StopCheck
 *-------------------------------------------------------------------------
 * PURPOSE : This function check go print next record or not from input
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - stop to get next record
 *           FALSE - go to get next record
 * NOTE    : the space means get next record.
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_BACKDOOR_StopCheck ();

#endif /*End of MLDSNP_BACKDOOR.H*/




