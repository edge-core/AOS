/*
 *   File Name: ipal_reflect.h
 *   Purpose:   TCP/IP shim layer(ipal) reflection interface implementation API
 *   Note:
 *   Create:    kh_shi     2008.11.4
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

#ifndef __IPAL_REFLECT_H
#define __IPAL_REFLECT_H

#include "sys_type.h"
#include "l_inet.h"

/*
 * INCLUDE FILE DECLARATIONS
 */


/*
 * NAMING CONST DECLARATIONS
 */

/*
 * MACRO FUNCTION DECLARATIONS
 */


/*
 * DATA TYPE DECLARATIONS
 */
typedef enum IPAL_ReflectType_E
{
    IPAL_REFLECT_TYPE_NEW_ADDR = 0,
    IPAL_REFLECT_TYPE_DEL_ADDR = 1,
    IPAL_REFLECT_TYPE_MAX
}IPAL_ReflectType_T;

typedef struct IPAL_ReflectNewAddr_S
{
    UI32_T          ifindex;
    L_INET_AddrIp_T ipaddr;
    UI8_T           flags;
}IPAL_ReflectNewAddr_T;

typedef struct IPAL_ReflectMesg_S
{
    UI32_T              type;  /* IPAL_ReflectType_T */
    union
    {
        IPAL_ReflectNewAddr_T addr;
    } u;
}IPAL_ReflectMesg_T;

/*
 * STATIC VARIABLE DECLARATIONS
 */


/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */
void IPAL_REFLECT_CreateTask(void);

/* FUNCTION NAME : IPAL_IF_GetSnmpStatisticByTpye
 * PURPOSE:
 *      Get IPv4 SNMP statistic counter
 *
 * INPUT:
 *      stat_type  -- One of snmp type in IPAL_SnmpStatisticType_T
 *
 * OUTPUT:
 *      value_p    -- retrieved value
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      1. Linux: Stored in /proc/net/snmp
 */
UI32_T IPAL_REFLECT_Read(IPAL_ReflectMesg_T *reflect_msg_p);


#endif /* end of __IPAL_REFLECT_H */
