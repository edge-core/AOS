/*-----------------------------------------------------------------------------
 * Module Name: dcbx_type.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the DCBX variables
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */
#ifndef DCBX_TYPE_H
#define DCBX_TYPE_H


#include "l_mm.h"
#include "sys_type.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "l_sort_lst.h"

/* when SomethingChanged */
#define DCBX_TYPE_MGR_CHANGED_TRUE                          1
#define DCBX_TYPE_MGR_CHANGED_FALSE                         0

/* DCBX gerneral return value */
#define DCBX_TYPE_RETURN_ERROR                              0
#define DCBX_TYPE_RETURN_OK                                 1
#define DCBX_TYPE_RETURN_MASTER_MODE_ERROR                  2

/* DCBX Init */
#define DCBX_TYPE_INIT_TX_FAIL                              0
#define DCBX_TYPE_INIT_TX_OK                                1
#define DCBX_TYPE_INIT_RX_FAIL                              0
#define DCBX_TYPE_INIT_RX_OK                                1

/* DCBX Port Status */
#define DCBX_TYPE_PORT_STATUS_DISABLE                       FALSE
#define DCBX_TYPE_PORT_STATUS_ENABLE                        TRUE

/* DCBX Port Mode */
#define DCBX_TYPE_PORT_MODE_MANUAL                          VAL_dcbxPortMode_manual
#define DCBX_TYPE_PORT_MODE_CFGSRC                          VAL_dcbxPortMode_configSource
#define DCBX_TYPE_PORT_MODE_AUTOUP                          VAL_dcbxPortMode_autoUp
#define DCBX_TYPE_PORT_MODE_AUTODOWN                        VAL_dcbxPortMode_autoDown


/* DCBX Events */
#define DCBX_TYPE_EVENT_NONE                                0
#define DCBX_TYPE_EVENT_DCBXDURCVD                          BIT_6
#define DCBX_TYPE_EVENT_ENTER_TRANSITION                    BIT_7
#define DCBX_TYPE_EVENT_SOMETHING_CHANGED_LOCAL             BIT_8
#define DCBX_TYPE_EVENT_ALL                                 0x0F


/* DCBX Timer */
#define DCBX_TYPE_TIMER_TICKS2SEC                           SYS_BLD_TICKS_PER_SECOND
#define DCBX_TYPE_TIME_UNIT                                 SYS_BLD_TICKS_PER_SECOND


/* DCBX config variable default value */
#define DCBX_TYPE_DEFAULT_PORT_STATUS                       DCBX_TYPE_PORT_STATUS_ENABLE
#define DCBX_TYPE_DEFAULT_PORT_MODE                         DCBX_TYPE_PORT_MODE_MANUAL

/* DCBX Asymmetric state machine state */
#define DCBX_TYPE_ASYM_DISABLE_STATE                    1
#define DCBX_TYPE_ASYM_INIT_STATE                           2
#define DCBX_TYPE_ASYM_RXRECOMMEND_STATE    3

/* DCBX Symmetric state machine state */
#define DCBX_TYPE_SYM_DISABLE_STATE                    1
#define DCBX_TYPE_SYM_INIT_STATE                           2
#define DCBX_TYPE_SYM_RXRECOMMEND_STATE    3

/* trace id definition when using L_MM to allocate buffer
 */
enum
{
    DCBX_TYPE_TRACE_ID_POM_GETREMOTESYSTEMDATA,
};

/* DCBX Asymmetric state machine event */
typedef enum
{
    DCBX_TYPE_EVENT_ASYM_NONE = 0,
    DCBX_TYPE_EVENT_ASYM_ENABLE,/* Begin */
    DCBX_TYPE_EVENT_ASYM_ADD_REMOTE_PARAM,/* Init to RxRecommend */
    DCBX_TYPE_EVENT_ASYM_REMOVE_REMOTE_PARAM,/* RxRecommend to Init */
    DCBX_TYPE_EVENT_ASYM_OPER_NOT_EQUAL_REMOTE_PARAM,/* RxRecommend to RxRecommend */
    DCBX_TYPE_EVENT_ASYM_OPER_NOT_EQUAL_LOCAL_ADMIN_PARAM,/* Init to Init */
    DCBX_TYPE_EVENT_ASYM_DISABLE,
}DCBX_TYPE_EVENT_ASYM_E;

/* DCBX Symmetric state machine event */
typedef enum
{
    DCBX_TYPE_EVENT_SYM_NONE = 0,
    DCBX_TYPE_EVENT_SYM_ENABLE,/* Begin */
    DCBX_TYPE_EVENT_SYM_ADD_REMOTE_PARAM,/* Init to RxRecommend */
    DCBX_TYPE_EVENT_SYM_REMOVE_REMOTE_PARAM,/* RxRecommend to Init */
    DCBX_TYPE_EVENT_SYM_OPER_NOT_EQUAL_REMOTE_PARAM,/* RxRecommend to RxRecommend */
    DCBX_TYPE_EVENT_SYM_OPER_NOT_EQUAL_LOCAL_ADMIN_PARAM,/* Init to Init */
    DCBX_TYPE_EVENT_SYM_DISABLE,
}DCBX_TYPE_EVENT_SYM_E;

/* DCBX Asymmetric state machine action */
typedef enum
{
    DCBX_TYPE_ASYM_ACTION_NONE = 1,
    DCBX_TYPE_ASYM_ACTION_INIT,
    DCBX_TYPE_ASYM_ACTION_APPLY_REMOTE
}DCBX_TYPE_ASYM_ACTION_E;

/* DCBX Symmetric state machine action */
typedef enum
{
    DCBX_TYPE_SYM_ACTION_NONE = 1,
    DCBX_TYPE_SYM_ACTION_INIT,
    DCBX_TYPE_SYM_ACTION_APPLY_REMOTE
}DCBX_TYPE_SYM_ACTION_E;

#endif
