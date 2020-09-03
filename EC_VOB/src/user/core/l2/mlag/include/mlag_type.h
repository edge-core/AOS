/* =============================================================================
 * MODULE NAME : MLAG_TYPE.H
 * PURPOSE     : Provide declarations for MLAG common constants, macros and data
 *               types.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#ifndef MLAG_TYPE_H
#define MLAG_TYPE_H

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

enum MLAG_TYPE_RETURN_CODE_E
{
    MLAG_TYPE_RETURN_OK = 0,
    MLAG_TYPE_RETURN_ERROR,     /* general error */
};

enum MLAG_TYPE_STATE_E
{
    MLAG_TYPE_STATE_INEXISTENT,
    MLAG_TYPE_STATE_DOWN,
    MLAG_TYPE_STATE_DORMANT,
    MLAG_TYPE_STATE_UP
};

#define MLAG_TYPE_EVENT_TIMER   BIT_0

#define MLAG_TYPE_GLOBAL_STATUS_ENABLED     1
#define MLAG_TYPE_GLOBAL_STATUS_DISABLED    2
#define MLAG_TYPE_DEFAULT_GLOBAL_STATUS     SYS_DFLT_MLAG_GLOBAL_STATUS

#define MLAG_TYPE_MIN_DOMAIN_ID_LEN     1
#define MLAG_TYPE_MAX_DOMAIN_ID_LEN     16

#define MLAG_TYPE_MIN_MLAG_ID   1
#define MLAG_TYPE_MAX_MLAG_ID   1000

#define MLAG_TYPE_MAX_NBR_OF_DOMAIN     1

/* if the number of MLAG is set to larger such that a LINK packet can not
 * all MLAGs, the code for sending LINK packet shall be modified
 */
#define MLAG_TYPE_MAX_NBR_OF_MLAG       SYS_ADPT_TOTAL_NBR_OF_LPORT

#define MLAG_TYPE_MAX_NBR_OF_REMOTE_FDB SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY

#define MLAG_TYPE_MLAG_LIST_SIZE    (MLAG_TYPE_MAX_MLAG_ID + 7) / 8

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#if __STDC_VERSION__ < 199901L
#if __GNUC__ >= 2
#define __func__ __FUNCTION__
#else
#define __func__ ""
#endif
#endif

#ifdef MLAG_UNIT_TEST
#define BACKDOOR_MGR_Printf printf
#endif

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

typedef struct
{
    char    domain_id[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1]; /* key */
    UI32_T  peer_link;
} MLAG_TYPE_DomainEntry_T;

typedef struct
{
    UI32_T  mlag_id;                                /* key */
    char    domain_id[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1];
    UI32_T  local_member;
    UI32_T  local_state;
    UI32_T  remote_state;
} MLAG_TYPE_MlagEntry_T;

#endif /* End of MLAG_TYPE_H */
