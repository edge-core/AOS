/* Module Name: SWDRV_MIM_TYPE.H
 * Purpose:
 *        type defintions for MAC-in-MAC
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *       2012/09/19  Wakka Tu    Create this file
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef _SWCTRL_MIM_TYPE_H
#define _SWCTRL_MIM_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "leaf_es3626a.h"
#include "leaf_sys.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SWCTRL_MIM_ISID_MIN     0       /* VAL_ */
#define SWCTRL_MIM_ISID_MAX     1000    /* SYS_ADPT */


/* TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T isid;

   /* output param for creation; input param for deletion
    *
    * for delection,
    * 1) fill all field except hw_idx or
    * 2) specify hw_idx and zero other field
    */
    UI32_T hw_idx;
} SWCTRL_MimServiceInfo_T;

typedef enum
{
    SWCTRL_MIM_PORT_TYPE_HWIDX,
    SWCTRL_MIM_PORT_TYPE_ACCESS,
    SWCTRL_MIM_PORT_TYPE_BACKBONE,
} SWCTRL_MimPortType_T;

typedef struct
{
    UI32_T isid;
    UI32_T lport;
    SWCTRL_MimPortType_T port_type;
    union {
        struct {
            UI32_T svid;
            UI32_T cvid;
        } access;
        struct {
            UI32_T egr_vid;
            UI8_T egr_src_addr[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T egr_dst_addr[SYS_ADPT_MAC_ADDR_LEN];
        } backbone;
    } ext;

   /* output param for creation; input param for deletion
    *
    * to delete port by hw_idx,
    * specify port_type = HWIDX and hw_idx of port to delete.
    */
    UI32_T hw_idx;
} SWCTRL_MimPortInfo_T;

#endif /* _SWCTRL_MIM_TYPE_H */

