/* MODULE NAME: security_backdoor.h
 * PURPOSE: 
 *  Implement security backdoor
 *
 * NOTES:
 *
 * History:
 *    2004/07/05 : mfhorng      Create this file
 *
 * Copyright(C)      Accton Corporation, 2004
 */
#ifndef SECURITY_BACKDOOR_H
#define SECURITY_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SECURITY_SUPPORT_ACCTON_BACKDOOR        TRUE    /* support security backdoor */


/* MACRO FUNCTION DECLARATIONS
 */
#define SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)         \
    if(flag&op)                                          \
    {                                                    \
        if(!(op&SECURITY_DEBUG_TYPE_NO_OPTION))          \
        {                                                \
            if(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE)\
                printf("%s(%d) : ", __FUNCTION__,__LINE__); \
        }

#define SECURITY_DEBUG_PRINT_MORE_END()         printf("\r\n");}

#define SECURITY_DEBUG_PRINT_PREFIX(prefix)              \
        printf("%s : ",prefix);                          \

#ifndef _MSC_VER
#define SECURITY_DEBUG_PRINT(flag,op, prefix, fmt, args...) \
    {                                                       \
        SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)            \
        if((!(op&SECURITY_DEBUG_TYPE_NO_OPTION) && !(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE))||!prefix)\
            SECURITY_DEBUG_PRINT_PREFIX(prefix)             \
        printf(fmt, ##args);                                \
        SECURITY_DEBUG_PRINT_MORE_END()                     \
    }
#else
#define SECURITY_DEBUG_PRINT(flag,op, prefix, fmt, ...)     \
    {                                                       \
        SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)            \
        if((!(op&SECURITY_DEBUG_TYPE_NO_OPTION) && !(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE))||!prefix)\
            SECURITY_DEBUG_PRINT_PREFIX(prefix)             \
        printf(fmt, ##args);                                \
        SECURITY_DEBUG_PRINT_MORE_END()                     \
    }
#endif

#define SECURITY_DEBUG_PRINT0(flag,op, prefix, msg){     \
        SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)         \
        if((!(op&SECURITY_DEBUG_TYPE_NO_OPTION) && !(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE))||!prefix)\
            SECURITY_DEBUG_PRINT_PREFIX(prefix)          \
        printf("%s",msg);                                \
        SECURITY_DEBUG_PRINT_MORE_END()                  \
    }

#define SECURITY_DEBUG_PRINT1(flag,op, prefix, msg, arg){\
        SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)         \
        if((!(op&SECURITY_DEBUG_TYPE_NO_OPTION) && !(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE))||!prefix)\
        SECURITY_DEBUG_PRINT_PREFIX(prefix)              \
        printf(msg, arg);                                \
        SECURITY_DEBUG_PRINT_MORE_END()                  \
    }

#define SECURITY_DEBUG_PRINT2(flag,op, prefix, msg, arg1,arg2){    \
        SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)         \
        if((!(op&SECURITY_DEBUG_TYPE_NO_OPTION) && !(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE))||!prefix)\
        SECURITY_DEBUG_PRINT_PREFIX(prefix)              \
        printf(msg, arg1,arg2);                          \
        SECURITY_DEBUG_PRINT_MORE_END()                  \
    }

#define SECURITY_DEBUG_PRINT3(flag,op, prefix, msg, arg1,arg2, arg3){    \
        SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)         \
        if((!(op&SECURITY_DEBUG_TYPE_NO_OPTION) && !(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE))||!prefix)\
        SECURITY_DEBUG_PRINT_PREFIX(prefix)              \
        printf(msg, arg1,arg2,arg3);                     \
        SECURITY_DEBUG_PRINT_MORE_END()                  \
    }

#define SECURITY_DEBUG_PRINT4(flag,op, prefix, msg, arg1,arg2, arg3, arg4){    \
        SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)         \
        if((!(op&SECURITY_DEBUG_TYPE_NO_OPTION) && !(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE))||!prefix)\
        SECURITY_DEBUG_PRINT_PREFIX(prefix)              \
        printf(msg, arg1,arg2,arg3,arg4);                \
        SECURITY_DEBUG_PRINT_MORE_END()                  \
    }

#define SECURITY_DEBUG_PRINT5(flag,op, prefix, msg, arg1,arg2, arg3, arg4, arg5){    \
        SECURITY_DEBUG_PRINT_MORE_BEGIN(flag,op)         \
        if((!(op&SECURITY_DEBUG_TYPE_NO_OPTION) && !(flag&SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE))||!prefix)\
        SECURITY_DEBUG_PRINT_PREFIX(prefix)              \
        printf(msg, arg1,arg2,arg3,arg4,arg5);           \
        SECURITY_DEBUG_PRINT_MORE_END()                  \
    }

#define SECURITY_DEBUG_NULL                                               ((void)0)


/* DATA TYPE DECLARATIONS
 */
typedef enum SECURITY_DebugType_E
{
    /* basic
     */
    SECURITY_DEBUG_TYPE_NONE             = 0x00000000,
    SECURITY_DEBUG_TYPE_ERR              = 0x00000001, /*error*/
    SECURITY_DEBUG_TYPE_RST              = 0x00000002, /*result*/
    SECURITY_DEBUG_TYPE_IFO              = 0x00000004, /*information*/
    SECURITY_DEBUG_TYPE_TRC              = 0x00000008, /*trace*/
    SECURITY_DEBUG_TYPE_TMR              = 0x00000010, /*timer*/
    SECURITY_DEBUG_TYPE_NO_OPTION        = 0X01000000, /*show manual text only*/

    SECURITY_DEBUG_TYPE_ERR_NOOP         = SECURITY_DEBUG_TYPE_ERR | SECURITY_DEBUG_TYPE_NO_OPTION,
    SECURITY_DEBUG_TYPE_RST_NOOP         = SECURITY_DEBUG_TYPE_RST | SECURITY_DEBUG_TYPE_NO_OPTION,
    SECURITY_DEBUG_TYPE_IFO_NOOP         = SECURITY_DEBUG_TYPE_IFO | SECURITY_DEBUG_TYPE_NO_OPTION,
    SECURITY_DEBUG_TYPE_TRC_NOOP         = SECURITY_DEBUG_TYPE_TRC | SECURITY_DEBUG_TYPE_NO_OPTION,
    SECURITY_DEBUG_TYPE_TMR_NOOP         = SECURITY_DEBUG_TYPE_TMR | SECURITY_DEBUG_TYPE_NO_OPTION,

    SECURITY_DEBUG_TYPE_TRC_IFO          = SECURITY_DEBUG_TYPE_IFO | SECURITY_DEBUG_TYPE_TRC,
    SECURITY_DEBUG_TYPE_ERR_TRC          = SECURITY_DEBUG_TYPE_ERR | SECURITY_DEBUG_TYPE_IFO | SECURITY_DEBUG_TYPE_TRC,

    SECURITY_DEBUG_TYPE_TRC_IFO_NOOP     = SECURITY_DEBUG_TYPE_IFO | SECURITY_DEBUG_TYPE_TRC | SECURITY_DEBUG_TYPE_NO_OPTION,
    SECURITY_DEBUG_TYPE_ERR_TRC_NOOP     = SECURITY_DEBUG_TYPE_ERR | SECURITY_DEBUG_TYPE_IFO | SECURITY_DEBUG_TYPE_TRC | SECURITY_DEBUG_TYPE_NO_OPTION,

    /* optional
     */
    SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE = 0x02000000,

    SECURITY_DEBUG_TYPE_ALL              = 0xFFFFFFFF  /*all*/
} SECURITY_DebugType_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if (SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  SECURITY_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : security backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void SECURITY_Backdoor_CallBack();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SECURITY_Backdoor_Register_SubsysBackdoorFunc
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
void SECURITY_Backdoor_Register_SubsysBackdoorFunc(void);

BOOL_T SECURITY_BACKDOOR_Register(const char *show_name, UI32_T *reg_no_p);
BOOL_T SECURITY_BACKDOOR_IsOn(UI32_T reg_no);

#endif /* SECURITY_SUPPORT_ACCTON_BACKDOOR == TRUE */


#endif /* End of SECURITY_BACKDOOR_H */
