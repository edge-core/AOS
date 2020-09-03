/* MODULE NAME: VXLAN_TYPE.H
 * PURPOSE:
 *   Declares global constants or data types used by VXLAN.
 * NOTES:
 *
 * HISTORY:
 *   04/21/2015 -- Kelly Chen, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef VXLAN_TYPE_H
#define VXLAN_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#if(SYS_CPNT_DEBUG == TRUE)
#include "debug_type.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define VXLAN_TYPE_UDP_DST_PORT_MIN 1
#define VXLAN_TYPE_UDP_DST_PORT_MAX 65535
#define VXLAN_TYPE_VNI_ID_MAX 16777215
#define VXLAN_TYPE_VNI_ID_MIN 1
#define VXLAN_TYPE_VLAN_ID_MAX 4094
#define VXLAN_TYPE_VLAN_ID_MIN 1
#define VXLAN_TYPE_DFLT_UDP_DST_PORT 4789                                /* SYS_DFLT_OF_VXLAN_UDP_DST_PORT */
#define VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3  TRUE

/* VXLAN API return values */
enum
{
    VXLAN_TYPE_RETURN_ERROR    =   0,   /* general error */
    VXLAN_TYPE_RETURN_OK,
    VXLAN_TYPE_SRC_IP_NOT_FIND,
    VXLAN_TYPE_ROUTE_NOT_FIND,
    VXLAN_TYPE_DRIVER_ERROR,
    VXLAN_TYPE_VNI_NOT_MATCH,
    VXLAN_TYPE_VNI_NOT_EXIST,
    VXLAN_TYPE_VNI_EXIST,
    VXLAN_TYPE_ENTRY_EXISTED,
    VXLAN_TYPE_ENTRY_NOT_EXISTED,
    VXLAN_TYPE_HOST_NOT_FIND,
    VXLAN_TYPE_TABLE_FULL,
    VXLAN_TYPE_IP_INVALID,
    VXLAN_TYPE_ACCESS_PORT_NOT_FIND,
    VXLAN_TYPE_SRC_IF_NOT_FIND,
    VXLAN_TYPE_SRC_IF_IP_NOT_FIND
};

#if(SYS_CPNT_DEBUG == TRUE)
enum VXLAN_TYPE_DEBUG_FLAG_E
{
    VXLAN_TYPE_DEBUG_FLAG_NONE          =   DEBUG_TYPE_VXLAN_NONE,
    VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG  =   DEBUG_TYPE_VXLAN_DATABASE,
    VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG     =   DEBUG_TYPE_VXLAN_EVENT,
    VXLAN_TYPE_DEBUG_FLAG_VNI_MSG       =   DEBUG_TYPE_VXLAN_VNI,
    VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG      =   DEBUG_TYPE_VXLAN_VTEP,
    VXLAN_TYPE_DEBUG_FLAG_ALL           =   DEBUG_TYPE_VXLAN_ALL
};
#else
enum VXLAN_TYPE_DEBUG_FLAG_E
{
    VXLAN_TYPE_DEBUG_FLAG_NONE          =   0x00000000L,
    VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG  =   BIT_0,
    VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG     =   BIT_1,
    VXLAN_TYPE_DEBUG_FLAG_VNI_MSG       =   BIT_2,
    VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG      =   BIT_3,
    VXLAN_TYPE_DEBUG_FLAG_ALL           =   0xFFFFFFFFL
};
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#define VXLAN_TYPE_IS_L_PORT(logical_vxlan_port) ((logical_vxlan_port)>=SYS_ADPT_VXLAN_LOGICAL_PORT_BASE && (logical_vxlan_port)<SYS_ADPT_VXLAN_MAX_LOGICAL_PORT_ID)
#define VXLAN_TYPE_IS_R_PORT(real_vxlan_port) ((real_vxlan_port)>=SYS_ADPT_VXLAN_MIN_REAL_PORT_ID && (real_vxlan_port)<SYS_ADPT_VXLAN_MAX_REAL_PORT_ID)
/*Convert the specified real vxlan port in chip to logical vxlan port in internal OM.
 */
#define VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(real_vxlan_port,logical_vxlan_port)\
    (logical_vxlan_port = ((real_vxlan_port < SYS_ADPT_VXLAN_MIN_REAL_PORT_ID) ? 0 : (real_vxlan_port-(SYS_ADPT_VXLAN_MIN_REAL_PORT_ID) + (SYS_ADPT_VXLAN_LOGICAL_PORT_BASE))))
/*Convert the specified logical vxlan port in internal OM to real vxlan port in chip
 */
#define VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(logical_vxlan_port,real_vxlan_port)\
    (real_vxlan_port = ((VXLAN_TYPE_IS_L_PORT(logical_vxlan_port)) ? (logical_vxlan_port-(SYS_ADPT_VXLAN_LOGICAL_PORT_BASE) + (SYS_ADPT_VXLAN_MIN_REAL_PORT_ID)) : 0))

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


#endif /* End of VXLAN_TYPE_H */