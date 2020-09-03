/*
 *   File Name: ipal_ioctl.h
 *   Purpose:   TCP/IP shim layer(ipal) ioctl utility
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify         Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

#ifndef __IPAL_IOCTL_H
#define __IPAL_IOCTL_H

/*
 * INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "ipal_types.h"

/*
 * NAMING CONST DECLARATIONS
 */



/*
 * MACRO FUNCTION DECLARATIONS
 */

/*
 *  Name    IPAL_GetIfFlags
 *  Purpose Get the flags of an interface in TCP/IP stack
 *  Input
 *          UI8_T  *ifname
 *          UI16_T flags
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_Ioctl_GetIfFlags (const char *ifname, UI16_T *flags_p);

/*
 *  Name    IPAL_SetIfFlags
 *  Purpose Set the flags of an interface in TCP/IP stack
 *  Input
 *          UI8_T  *ifname
 *          UI16_T flags
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_Ioctl_SetIfFlags (const char *ifname, UI16_T flags);

/*
 *  Name    IPAL_UnsetIfFlags
 *  Purpose Unset the flags of an interface in TCP/IP stack
 *  Input
 *          UI16_T flags
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_Ioctl_UnSetIfFlags (const char *ifname, UI16_T flags);

/*
 *  Name    IPAL_GetIfMtu
 *  Purpose Get the MTU of an interface in TCP/IP stack
 *  Input   char *ifname
 *          UI32_T mtu
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_Ioctl_GetIfMtu (const char *ifname, UI32_T *mtu_p);

/*
 *  Name    IPAL_SetIfMtu
 *  Purpose Set the MTU of an interface in TCP/IP stack
 *  Input   char *ifname
 *          UI32_T mtu
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_Ioctl_SetIfMtu (const char *ifname, UI32_T mtu);

/*
 * DATA TYPE DECLARATIONS
 */
UI32_T IPAL_Ioctl_SetIfMac (const char *ifname, const UI8_T *mac_addr);

/*
 * get  vlan interface ip address
 */
UI32_T IPAL_Ioctl_GetIfIPaddr (char *ifname, L_INET_AddrIp_T *ipaddr_p);

UI32_T IPAL_Ioctl_SetIfBandwidth (const char *ifname, UI32_T bandwidth);

#endif /* end of __IPAL_IOCTL_H */

