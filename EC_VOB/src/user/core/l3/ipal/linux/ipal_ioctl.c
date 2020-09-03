/*
 *   File Name: ipal_ioctl.c
 *   Purpose:   TCP/IP shim layer(ipal) ioctl interface implementation
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify         Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

/*
 * INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <errno.h>
#include <linux/if_arp.h>
#include "sys_type.h"
#include "sysfun.h"

#include "l_prefix.h"

#include "ipal_debug.h"
#include "ipal_types.h"

/*
 * NAMING CONST DECLARATIONS
 */



/*
 * MACRO FUNCTION DECLARATIONS
 */


/*
 * DATA TYPE DECLARATIONS
 */


/*
 * STATIC VARIABLE DECLARATIONS
 */



/*
 * LOCAL SUBPROGRAM DECLARATIONS
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
UI32_T IPAL_Ioctl_GetIfFlags (const char *ifname, UI16_T *flags_p)
{
    int    fd = -1;
    UI32_T ret = IPAL_RESULT_OK;

    struct ifreq ifreq;

    if(NULL == ifname)
        return IPAL_RESULT_FAIL;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Create Socket Fail, %s.\r\n", strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    strncpy(ifreq.ifr_ifrn.ifrn_name, (const char *)ifname, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFFLAGS, (caddr_t)&ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Get Interface %s Flags Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    *flags_p = ifreq.ifr_flags;

errout:
    if (fd > 0)
        close(fd);

    return ret;
}

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
UI32_T IPAL_Ioctl_SetIfFlags (const char *ifname, UI16_T flags)
{
    int    fd = -1;
    UI32_T ret = IPAL_RESULT_OK;

    struct ifreq ifreq;

    if(NULL == ifname)
        return IPAL_RESULT_FAIL;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Create Socket Fail, %s.\r\n", strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    strncpy(ifreq.ifr_ifrn.ifrn_name, (const char *)ifname, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFFLAGS, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Get Interface %s Flags Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    ifreq.ifr_flags |= flags;
    if (ioctl(fd, SIOCSIFFLAGS, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Set Interface %s Flags Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

errout:
    if (fd > 0)
        close(fd);

    return ret;
}

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
UI32_T IPAL_Ioctl_UnSetIfFlags (const char *ifname, UI16_T flags)
{
    int    fd = -1;
    UI32_T ret = IPAL_RESULT_OK;

    struct ifreq ifreq;

    if(NULL == ifname)
        return IPAL_RESULT_FAIL;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Create Socket Fail, %s.\r\n", strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    strncpy(ifreq.ifr_ifrn.ifrn_name, (const char *)ifname, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFFLAGS, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Get Interface %s Flags Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    ifreq.ifr_flags &= ~flags;
    if (ioctl(fd, SIOCSIFFLAGS, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Unset Interface %s Flags Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

errout:
    if (fd > 0)
        close(fd);

    return ret;
}

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
UI32_T IPAL_Ioctl_GetIfMtu (const char *ifname, UI32_T *mtu_p)
{
    int    fd = -1;
    UI32_T ret = IPAL_RESULT_OK;

    struct ifreq ifreq;

    if(NULL == ifname)
        return IPAL_RESULT_FAIL;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Create Socket Fail, %s.\r\n", strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    strncpy(ifreq.ifr_ifrn.ifrn_name, (const char *)ifname, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFMTU, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Get Interface %s MTU Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    *mtu_p = ifreq.ifr_mtu;

errout:
    if (fd > 0)
        close(fd);

    return ret;
}

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
UI32_T IPAL_Ioctl_SetIfMtu (const char *ifname, UI32_T mtu)
{
    int    fd = -1;
    UI32_T ret = IPAL_RESULT_OK;

    struct ifreq ifreq;

    if(NULL == ifname)
        return IPAL_RESULT_FAIL;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Create Socket Fail, %s.\r\n", strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    strncpy(ifreq.ifr_ifrn.ifrn_name, (const char *)ifname, IFNAMSIZ);
    ifreq.ifr_mtu = mtu;
    if (ioctl(fd, SIOCSIFMTU, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Set Interface %s MTU Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

errout:
    if (fd > 0)
        close(fd);

    return ret;
}

UI32_T IPAL_Ioctl_SetIfMac (const char *ifname, const UI8_T *mac_addr)
{
    int    fd = -1;
    UI32_T ret = IPAL_RESULT_OK;
    UI16_T flags = 0;

    struct ifreq ifreq;

    if(NULL == ifname)
        return IPAL_RESULT_FAIL;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Create Socket Fail, %s.\r\n", strerror(errno));
        return IPAL_RESULT_FAIL;
    }

    if (IPAL_Ioctl_GetIfFlags(ifname, &flags) != IPAL_RESULT_OK)
    {
        IPAL_DEBUG_PRINT ("Get Interface Flags Fail, %s.\r\n", strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    if (flags & IFF_UP)
        IPAL_Ioctl_UnSetIfFlags(ifname, IFF_UP);

    memset(&ifreq, 0x0, sizeof(struct ifreq));
    //ifreq.ifr_ifru.ifru_hwaddr.sa_family = ARPHRD_ETHER;
    ifreq.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    //strncpy(ifreq.ifr_ifrn.ifrn_name, (const char *)ifname, IFNAMSIZ);
    strncpy(ifreq.ifr_name, (const char *)ifname, sizeof(ifreq.ifr_name));
    //memcpy(ifreq.ifr_ifru.ifru_hwaddr.sa_data, mac_addr, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(ifreq.ifr_hwaddr.sa_data, mac_addr, SYS_ADPT_MAC_ADDR_LEN);

    if (ioctl(fd, SIOCSIFHWADDR, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Set Interface %s MAC Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

errout:
    close(fd);

    if (flags & IFF_UP)
        IPAL_Ioctl_SetIfFlags(ifname, IFF_UP);

    return ret;
}

UI32_T IPAL_Ioctl_GetIfIPaddr (char *ifname, L_INET_AddrIp_T *ipaddr_p)
{
    int    fd = -1;
    UI32_T ret = IPAL_RESULT_OK;
    struct ifreq ifreq;
    int family_type;

    if(NULL == ifname)
        return IPAL_RESULT_FAIL;

    switch (ipaddr_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            family_type = AF_INET;
            ipaddr_p->addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            break;
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            family_type = AF_INET6;
            ipaddr_p->addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            break;
        default:
            return IPAL_RESULT_FAIL;
    }

    fd = socket(family_type, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Create Socket Fail, %s.\r\n", strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    ifreq.ifr_ifru.ifru_addr.sa_family = family_type;
    strncpy(ifreq.ifr_ifrn.ifrn_name, (const char *)ifname, IFNAMSIZ);

    if (ioctl(fd, SIOCGIFADDR, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Get Interface %s IP address Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    memcpy(ipaddr_p->addr, &ifreq.ifr_addr, ipaddr_p->addrlen);
    ipaddr_p->preflen = 0;

errout:
    if (fd > 0)
        close(fd);

    return ret;
}

UI32_T IPAL_Ioctl_SetIfBandwidth (const char *ifname, UI32_T bandwidth)
{
#if 0
    int    fd = -1;
    UI32_T ret = IPAL_RESULT_OK;
    int    err = 0;
    struct ifreq ifreq;

    if(NULL == ifname)
        return IPAL_RESULT_FAIL;

    fd = socket (AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Create Socket Fail, %s.\r\n", strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

    ifreq.ifr_ifru.ifru_hwaddr.sa_family = AF_INET;
    strncpy(ifreq.ifr_ifrn.ifrn_name, ifname, IFNAMSIZ);
    ifreq.ifr_bandwidth = bandwidth;

    if (ioctl(fd, SIOCSIF, (caddr_t) &ifreq) < 0)
    {
        IPAL_DEBUG_PRINT ("Set Interface %s MAC Fail, %s.\r\n", ifname, strerror(errno));
        ret = IPAL_RESULT_FAIL;
        goto errout;
    }

errout:
    if (fd > 0)
        close(fd);

    return ret;
#endif

    return IPAL_RESULT_FAIL;
}

