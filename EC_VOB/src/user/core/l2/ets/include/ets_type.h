/* MODULE NAME: ets_type.h
 * PURPOSE:
 *   Declarations of variables/structure used for ETS
 *   (IEEE Std 802.1Qaz - Enhanced Transmission Selection).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef _ETS_TYPE_H
#define _ETS_TYPE_H

#define ETS_TYPE_BUILD_LINUX        TRUE

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "leaf_es3626a.h"
#include "leaf_sys.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* TYPE DECLARATIONS
 */
typedef enum {
    ETS_TYPE_MODE_OFF =0, /*have to be 0 to match SYS_DFLT_ETS_MODE*/
    ETS_TYPE_MODE_USER,
    ETS_TYPE_MODE_AUTO,
} ETS_TYPE_MODE_T;

typedef enum {
    ETS_TYPE_DB_OPER,
    ETS_TYPE_DB_CONFIG,
} ETS_TYPE_DB_T;

typedef enum {
    ETS_TYPE_TSA_SP   =0,     /*StrictPriority*/
    ETS_TYPE_TSA_CBS  =1,     /*CreditBasedShaper*/
    ETS_TYPE_TSA_ETS  =2,     /*EnhancedTransmission*/
    ETS_TYPE_TSA_VS   =255,   /*VendorSpecific*/
} ETS_TYPE_TSA_T;          /*Traffic Selection Algorithm*/

typedef struct
{
    ETS_TYPE_TSA_T tsa[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];       /*Traffic Selection Algorithm*/
    UI32_T priority_assign[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];
    UI32_T tc_weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];
} ETS_TYPE_PortEntry_T;

typedef struct
{
    ETS_TYPE_MODE_T mode[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    ETS_TYPE_PortEntry_T port_entry[SYS_ADPT_TOTAL_NBR_OF_LPORT];
} ETS_TYPE_INFO_T;

enum ETS_TYPE_RETURN_CODE_E
{
    ETS_TYPE_RETURN_OK = 0,             /* OK, Successful, Without any Error */
    ETS_TYPE_RETURN_INPUT_ERR,          /* Invalid input parameter */
    ETS_TYPE_RETURN_CHIP_ERROR,         /* R/W CHIP FAIL */
    ETS_TYPE_RETURN_PORTNO_OOR,         /* Port number Out Of Range*/
    ETS_TYPE_RETURN_MASTER_MODE_ERROR,  /* Only valid on Master mode*/
    ETS_TYPE_RETURN_ILL_MODE,           /* UI set only on "on" mode, DCBX on "auto" mode*/
    ETS_TYPE_RETURN_OTHERS              /* Other Errors*/
};
#endif /* End of _ETS_TYPE_H */


