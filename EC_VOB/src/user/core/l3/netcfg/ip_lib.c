/* Module Name: IP_LIB.C
 * Purpose:
 *      IP_LIB provides some library for network layer, include loopback IP, zero IP,
 *      overlapping...
 *
 * Notes:
 *      None.
 *
 * History:
 *       Date       --  Modifier,   Reason
 *  0.1 2002.06.23  --  William,    First Created.
 *      2007.7      --  peter_yu    Port to Linux Platform
 *
 * Copyright(C)      Accton Corporation, 2002-2007.
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <string.h>
#include "ip_lib.h"
#include "l_stdlib.h" /* for L_STDLIB_Ntoh32 */

/* NAMING CONSTANT DECLARATIONS
 */
#define IP_CLASS_A_MAX                      127
#define IP_CLASS_B_MAX                      191
#define IP_EXT_ADD                          224

#define IP_LIB_LOOPBACK_NETWORK_BYTE0               0x7f        /* 127 of 127.x.x.x */
#define IP_LIB_CLASS_B_RESERVE_NETWORK_BYTE0        0xbf        /* 191 of 191.255.xx.xx */
#define IP_LIB_CLASS_B_RESERVE_NETWORK_BYTE1        0xff        /* 255 of 191.255.xx.xx */
#define IP_LIB_CLASS_C_RESERVE_NETWORK_1_BYTE0      0xc0        /* 192 of 192.00.00.xx */

#define IP_LIB_CLASS_C_RESERVE_NETWORK_2_BYTE0      0xdf        /* 223 of 223.255.255.xx */
#define IP_LIB_CLASS_C_RESERVE_NETWORK_2_BYTE1      0xff        /* 1st 255 of 223.255.255.xx */
#define IP_LIB_CLASS_C_RESERVE_NETWORK_2_BYTE2      0xff        /* 2nd 255 of 223.255.255.xx */

#define IP_LIB_MULTICAST_NETWORK_BYTE0              0xe0        /*  224 of 224.0.0.0 */
#define IP_LIB_TEST_NETWORK_BYTE0                   0xf0        /*  240 of 240.0.0.0 */


/* MACRO FUNCTION DECLARATIONS
 */
#define IP_LIB_CNV_SKTYPE_BIT(x)

#define IP_LIB_CHKAPP_APPSKPT_INFO(sk_port, bmp, app)       \
            {.skt_type_bmp  = IP_LIB_SKTTYPE_##bmp##_BIT,   \
             .skt_port      = sk_port,                      \
             .allow_app     = app},

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static int IP_LIB_LocalCmpChkAppScoketPortCmp(const void *key_p, const void *member_p);

/* STATIC VARIABLE DECLARATIONS
 */
static UI8_T ip_lib_cider_to_mask_table[][4] =
{
    {0x0 , 0x0 , 0x0 , 0x0 },  // 0x00000000,  /* 0 */
    {0x80, 0x0 , 0x0 , 0x0 },  // 0x80000000,  /* 1 */
    {0xc0, 0x0 , 0x0 , 0x0 },  // 0xc0000000,  /* 2 */
    {0xe0, 0x0 , 0x0 , 0x0 },  // 0xe0000000,  /* 3 */
    {0xf0, 0x0 , 0x0 , 0x0 },  // 0xf0000000,  /* 4 */

    {0xf8, 0x0, 0x0, 0x0},  //0xf8000000,   /* 5 */
    {0xfc, 0x0, 0x0, 0x0},  //0xfc000000,   /* 6 */
    {0xfe, 0x0, 0x0, 0x0},  //0xfe000000,   /* 7 */
    {0xff, 0x0, 0x0, 0x0},  //0xff000000,   /* 8 */

    {0xff, 0x80, 0x0, 0x0}, //0xff800000,   /* 9 */
    {0xff, 0xc0, 0x0, 0x0}, //0xffc00000,   /* 10 */
    {0xff, 0xe0, 0x0, 0x0}, //0xffe00000,   /* 11 */
    {0xff, 0xf0, 0x0, 0x0}, //0xfff00000,   /* 12 */

    {0xff, 0xf8, 0x0, 0x0}, //0xfff80000,  /* 13 */
    {0xff, 0xfc, 0x0, 0x0}, //0xfffc0000,  /* 14 */
    {0xff, 0xfe, 0x0, 0x0}, //0xfffe0000,  /* 15 */
    {0xff, 0xff, 0x0, 0x0}, //0xffff0000,  /* 16 */

    {0xff, 0xff, 0x80, 0x0}, //0xffff8000,  /* 17 */
    {0xff, 0xff, 0xc0, 0x0}, //0xffffc000,  /* 18 */
    {0xff, 0xff, 0xe0, 0x0}, //0xffffe000,  /* 19 */
    {0xff, 0xff, 0xf0, 0x0}, //0xfffff000,  /* 20 */

    {0xff, 0xff, 0xf8, 0x0}, //0xfffff800,  /* 21 */
    {0xff, 0xff, 0xfc, 0x0}, //0xfffffc00,  /* 22 */
    {0xff, 0xff, 0xfe, 0x0}, //0xfffffe00,  /* 23 */
    {0xff, 0xff, 0xff, 0x0}, //0xffffff00,  /* 24 */

    {0xff, 0xff, 0xff, 0x80}, //0xffffff80,  /* 25 */
    {0xff, 0xff, 0xff, 0xc0}, //0xffffffc0,  /* 26 */
    {0xff, 0xff, 0xff, 0xe0}, //0xffffffe0,  /* 27 */
    {0xff, 0xff, 0xff, 0xf0}, //0xfffffff0,  /* 28 */

    {0xff, 0xff, 0xff, 0xf8}, //0xfffffff8,  /* 29 */
    {0xff, 0xff, 0xff, 0xfc}, //0xfffffffc,  /* 30 */
    {0xff, 0xff, 0xff, 0xfe}, //0xfffffffe,  /* 31 */
    {0xff, 0xff, 0xff, 0xff}  //0xffffffff,  /* 32 */
};

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : IP_LIB_Init
 * PURPOSE:
 *      Initialize IP_LIB used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void    IP_LIB_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
}   /*  end of IP_LIB_Init  */


/* FUNCTION NAME : IP_LIB_IsZeroNetwork
 * PURPOSE:
 *      Is the ip belonging to zero network ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip belong zero network.
 *      FALSE -- ip not belong no zero network.
 *
 * NOTES:
 *      1. It's class A subnet : 00.xx.xx.xx.
 */
BOOL_T IP_LIB_IsZeroNetwork(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    if (0== ip_address[0])
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
} /* IP_LIB_IsZeroNetwork */


/* FUNCTION NAME : IP_LIB_IsLoopBackIp
 * PURPOSE:
 *      Is the ip a loop back IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip belong loopback network.
 *      FALSE -- ip not belong no loopback network.
 *
 * NOTES:
 *      1. It's class A subnet : 127.xx.xx.xx
 */
BOOL_T IP_LIB_IsLoopBackIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    if(IP_LIB_LOOPBACK_NETWORK_BYTE0 == ip_address[0])
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
} /* IP_LIB_IsLoopBackIp */


/* FUNCTION NAME : IP_LIB_IsClassBReserveIp
 * PURPOSE:
 *      Is the ip in reserved Class B IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is Class B reserved ip.
 *      FALSE -- ip not in Class B reserved ip.
 *
 * NOTES:
 *      1. It's class B subnet : 191.255.xx.xx/16
 */
BOOL_T IP_LIB_IsClassBReserveIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    if (IP_LIB_CLASS_B_RESERVE_NETWORK_BYTE0 == ip_address[0] &&
        IP_LIB_CLASS_B_RESERVE_NETWORK_BYTE1 == ip_address[1] )
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
} /* IP_LIB_IsClassBReserveIp */


/* FUNCTION NAME : IP_LIB_IsClassCReservedIp
 * PURPOSE:
 *      Is the ip in reserved Class C IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is Class C reserved ip.
 *      FALSE -- ip not in Class C reserved ip.
 *
 * NOTES:
 *      1. It's class C subnet : 192.00.00.xx/24, 223.255.255.0/24
 */
BOOL_T IP_LIB_IsClassCReservedIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY
     */
    if( (IP_LIB_CLASS_C_RESERVE_NETWORK_1_BYTE0 == ip_address[0]) ||
        ((IP_LIB_CLASS_C_RESERVE_NETWORK_2_BYTE0 == ip_address[0]) && (IP_LIB_CLASS_C_RESERVE_NETWORK_2_BYTE1 == ip_address[1]))
      )
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }

} /* IP_LIB_IsClassCReservedIp */


/* FUNCTION NAME : IP_LIB_IsTestingIp
 * PURPOSE:
 *      Is the ip is in reserved test-IP.
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a reserved test-ip.
 *      FALSE -- ip not a reserved test-ip.
 *
 * NOTES:
 *      1. It's reserved for testing : 240.00.00.00 ~ 255.255.255.254
 */
BOOL_T IP_LIB_IsTestingIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    if ((IP_LIB_TEST_NETWORK_BYTE0==(ip_address[0] & IP_LIB_TEST_NETWORK_BYTE0)) && (FALSE == IP_LIB_IsBroadcastIp(ip_address)))
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}   /*  end of IP_LIB_IsTestingIp   */


/* FUNCTION NAME : IP_LIB_IsBroadcastIp
 * PURPOSE:
 *      Is the ip is broadcast IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a broadcast IP.
 *      FALSE -- ip not a broadcast IP.
 *
 * NOTES:
 *      1. It's reserved for testing :  255.255.255.255
 */
BOOL_T IP_LIB_IsBroadcastIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    if( (IP_LIB_BROADCAST_IP_BYTE == ip_address[0]) &&
        (IP_LIB_BROADCAST_IP_BYTE == ip_address[1]) &&
        (IP_LIB_BROADCAST_IP_BYTE == ip_address[2]) &&
        (IP_LIB_BROADCAST_IP_BYTE == ip_address[3]))
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}   /*  end of IP_LIB_IsBroadcastIp */

/* FUNCTION NAME : IP_LIB_IsValidForNetworkInterface
 * PURPOSE:
 *      Is the ip can used as network interface's IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip can be used as network interface ip.
 *      FALSE -- the ip can't be network interface ip.
 *
 * NOTES:
 *      1. This IP can't in zero network, loop back ip, multicast ip, broadcast IP.
 *         But can be a reserved ip for private network inner IP.
 */
BOOL_T IP_LIB_IsValidForNetworkInterface(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    UI8_T  mask_class_a[SYS_ADPT_IPV4_ADDR_LEN] = {0xff, 00, 00, 00};
    UI8_T  mask_class_b[SYS_ADPT_IPV4_ADDR_LEN] = {0xff, 0xff, 00, 00};
    UI8_T  mask_class_c[SYS_ADPT_IPV4_ADDR_LEN] = {0xff, 0xff, 0xff, 00};

    /* LOCAL VARIABLES DEFINITION
     */
    UI8_T  b_addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI8_T  mask[SYS_ADPT_IPV4_ADDR_LEN];
    UI8_T  network_id[SYS_ADPT_IPV4_ADDR_LEN];

    /* BODY
     */
    if ((IP_LIB_IsLoopBackIp (ip_address) == TRUE) ||
        (IP_LIB_IsZeroNetwork(ip_address) == TRUE) ||
        (IP_LIB_IsBroadcastIp(ip_address) == TRUE))
    {
        return (FALSE);
    }

    /*  find the mask if with spec. */
    if (IP_LIB_IsIpInClassA(ip_address) == TRUE)
        memcpy(mask, mask_class_a, sizeof(mask));
    else if (IP_LIB_IsIpInClassB(ip_address) == TRUE)
        memcpy(mask, mask_class_b, sizeof(mask));
    else if (IP_LIB_IsIpInClassC(ip_address) == TRUE)
        memcpy(mask, mask_class_c, sizeof(mask));
    else
    {
        return (FALSE);
        /* For IP address not in Class A,B,C, it can not be used for network interface address */
    }


    /*  network b'cast ip   */
    if (IP_LIB_GetSubnetBroadcastIp(ip_address, mask, b_addr)==IP_LIB_OK)
    {
        if (0 == memcmp(b_addr, ip_address, SYS_ADPT_IPV4_ADDR_LEN))
        {
            return (FALSE);
        }
    }
    /*  network ID  */
    network_id[0] = ip_address[0] & mask[0];
    network_id[1] = ip_address[1] & mask[1];
    network_id[2] = ip_address[2] & mask[2];
    network_id[3] = ip_address[3] & mask[3];

    if (0 == memcmp(network_id, ip_address, SYS_ADPT_IPV4_ADDR_LEN))
    {
        return (FALSE);
    }

    return (TRUE);
}   /*  end of IP_LIB_IsValidForNetworkInterface    */

/* FUNCTION NAME : IP_LIB_IsValidForIpConfig
 * PURPOSE:
 *      Check if the ip address is valid for ip configuration.
 *
 * INPUT:
 *      ip_address  -- ip address
 *      mask        -- mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      IP_LIB_OK
 *      IP_LIB_INVALID_IP_LOOPBACK_IP
 *      IP_LIB_INVALID_IP_ZERO_NETWORK
 *      IP_LIB_INVALID_IP_BROADCAST_IP
 *      IP_LIB_INVALID_IP_IN_CLASS_D
 *      IP_LIB_INVALID_IP_IN_CLASS_E
 *      IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR
 *      IP_LIB_INVALID_IP_SUBNET_NETWORK_ID
 *
 * NOTES:
 *      1. This address could not in zero network, or be loopback, multicast, broadcast address.
 *      2. Check subnet network id or subnet broadcast address.
 *      3. return value is UI32_T.
 */
UI32_T IP_LIB_IsValidForIpConfig(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    UI8_T  bcast_addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI8_T  network_id[SYS_ADPT_IPV4_ADDR_LEN];

    /* BODY
     */
    if (IP_LIB_IsLoopBackIp(ip_address) == TRUE)
    {
        return IP_LIB_INVALID_IP_LOOPBACK_IP; 
    }
    else if (IP_LIB_IsZeroNetwork(ip_address) == TRUE)
    {
        return IP_LIB_INVALID_IP_ZERO_NETWORK;
    }
    else if (IP_LIB_IsBroadcastIp(ip_address) == TRUE)
    {
        return IP_LIB_INVALID_IP_BROADCAST_IP;
    }
    else if (IP_LIB_IsIpInClassD(ip_address))
    {
        return IP_LIB_INVALID_IP_IN_CLASS_D; 
    }
    else if (IP_LIB_IsIpInClassE(ip_address))
    {
        return IP_LIB_INVALID_IP_IN_CLASS_E;
    }
    /*  network b'cast ip   */
    if (IP_LIB_GetSubnetBroadcastIp(ip_address, mask, bcast_addr)==IP_LIB_OK)
    {
        if(!memcmp(bcast_addr, ip_address, SYS_ADPT_IPV4_ADDR_LEN))
        {
            return IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR;
        }
    }

    /*  network ID  */
    if (IP_LIB_GetNetworkID(ip_address, mask, network_id) == IP_LIB_OK) 
    {
        if(!memcmp(network_id, ip_address,SYS_ADPT_IPV4_ADDR_LEN))
        {
            return IP_LIB_INVALID_IP_SUBNET_NETWORK_ID;
        }
    }

    return IP_LIB_OK;
} 

/* FUNCTION NAME : IP_LIB_IsValidForRemoteIp
 * PURPOSE:
 *      Check if the ip address is valid for remote server ip.
 *
 * INPUT:
 *      ip_address  -- ip address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      IP_LIB_OK
 *      IP_LIB_INVALID_IP_LOOPBACK_IP
 *      IP_LIB_INVALID_IP_ZERO_NETWORK
 *      IP_LIB_INVALID_IP_BROADCAST_IP
 *      IP_LIB_INVALID_IP_IN_CLASS_D
 *      IP_LIB_INVALID_IP_IN_CLASS_E
 *
 * NOTES:
 *      1. This address could not in zero network, or be loopback, multicast, broadcast address.
 *      2. return value is UI32_T.
 *      3. we don't check with ip/mask of DUT's IP interface (RIF).
 */
UI32_T IP_LIB_IsValidForRemoteIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    if (IP_LIB_IsLoopBackIp(ip_address) == TRUE)
    {
        return IP_LIB_INVALID_IP_LOOPBACK_IP; 
    }
    else if (IP_LIB_IsZeroNetwork(ip_address) == TRUE)
    {
        return IP_LIB_INVALID_IP_ZERO_NETWORK;
    }
    else if (IP_LIB_IsBroadcastIp(ip_address) == TRUE)
    {
        return IP_LIB_INVALID_IP_BROADCAST_IP;
    }
    else if (IP_LIB_IsIpInClassD(ip_address) == TRUE)
    {
        return IP_LIB_INVALID_IP_IN_CLASS_D; 
    }
    else if (IP_LIB_IsIpInClassE(ip_address) == TRUE)
    {
        return IP_LIB_INVALID_IP_IN_CLASS_E;
    }

    return IP_LIB_OK;
} 

/* FUNCTION NAME : IP_LIB_IsMulticastIp
 * PURPOSE:
 *      Is the ip a multicast IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a multicast IP.
 *      FALSE -- ip not a multicast IP.
 *
 * NOTES:
 *      1. This IP is in 224.00.00.00 ~ 239.255.255.255.
 *
 */
BOOL_T IP_LIB_IsMulticastIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    if (IP_LIB_MULTICAST_NETWORK_BYTE0 == (ip_address[0] & 0xf0))
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}   /*  end of IP_LIB_IsMulticastIp */


/* FUNCTION NAME : IP_LIB_GetSubnetBroadcastIp
 * PURPOSE:
 *      Get subnet direct broadcast IP, which host is all 1.
 *
 * INPUT:
 *      ip_address  -- the ip of subnet.
 *      ip_mask     -- mask of subnet.
 *
 * OUTPUT:
 *      bcast_ip   -- the broadcast ip of subnet.
 *
 * RETURN:
 *      IP_LIB_OK - successfully get bcast ip.
 *      IP_LIB_INVALID_ARG -- invalid bcast_ip address or subnet (ip,mask).
 *
 * NOTES:
 *      None.
 *
 */
UI32_T IP_LIB_GetSubnetBroadcastIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN], UI8_T bcast_ip[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY
     */
    bcast_ip[0] = (ip_address[0] & ip_mask[0])|(IP_LIB_BROADCAST_IP_BYTE & ~ip_mask[0]);
    bcast_ip[1] = (ip_address[1] & ip_mask[1])|(IP_LIB_BROADCAST_IP_BYTE & ~ip_mask[1]);
    bcast_ip[2] = (ip_address[2] & ip_mask[2])|(IP_LIB_BROADCAST_IP_BYTE & ~ip_mask[2]);
    bcast_ip[3] = (ip_address[3] & ip_mask[3])|(IP_LIB_BROADCAST_IP_BYTE & ~ip_mask[3]);

    return (IP_LIB_OK);
}   /*  end of IP_LIB_GetSubnetBroadcastIp  */


/* -------------------------------------------------------------------------
 * FUNCTION NAME: IP_LIB_MaskToCidr
 * -------------------------------------------------------------------------
 * PURPOSE:  Translate the mask to cidr ( prefix_length )
 * INPUT:    mask
 * OUTPUT:   none.
 * RETURN:   cidr ( prefix_length)
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T IP_LIB_MaskToCidr(UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL VARIABLES DEFINITION
     */
    int i, j;
    UI32_T cidr=0;
    UI8_T mask_byte; /* one byte in the mask value */

    /* BODY
     */
    for(i=0; i<SYS_ADPT_IPV4_ADDR_LEN; i++)
    {
        mask_byte = mask[i];

        for(j=0; j<8;j++)
        {
            if( mask_byte & 0x80)
            {
                cidr++;
                mask_byte = mask_byte << 1;
            }
            else
            {
                break;
            }
        } /* j */
    } /* i */

    return cidr;
}  /* End of IP_LIB_IP_LIB_MaskToCidr */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: IP_LIB_CidrToMask
 * -------------------------------------------------------------------------
 * PURPOSE:  Translate the prefix length to mask
 * INPUT:    prefix_length
 * OUTPUT:   mask.
 * RETURN:   none
 * NOTES:
 * -------------------------------------------------------------------------*/
void IP_LIB_CidrToMask(UI32_T prefix_length, UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
    if (prefix_length > 32)
    {
        return;
    }

    memcpy(mask, ip_lib_cider_to_mask_table[prefix_length], SYS_ADPT_IPV4_ADDR_LEN);
    return;
}



/* FUNCTION NAME : IP_LIB_IsIpInClassA
 * PURPOSE:
 *      Check the IP is in Class A or not ?
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class A; 1~127
 *      FALSE -- no, not in Class A.
 *
 * NOTES:
 *      1. 0 (0x0)
 *
 */
BOOL_T IP_LIB_IsIpInClassA(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    if((0 == ip_address[0]) &&
       (0 == ip_address[1]) &&
       (0 == ip_address[2]) &&
       (0 == ip_address[3]))
    {
        return (FALSE);
    }

    if(ip_address[0] <= IP_CLASS_A_MAX)
    {
        return TRUE;
    }

    return FALSE;
}   /*  end of IP_LIB_IsIpInClassA  */


/* FUNCTION NAME : IP_LIB_IsIpInClassB
 * PURPOSE:
 *      Check the IP is in Class B or not ?
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class B; 128~191
 *      FALSE -- no, not in Class B.
 *
 * NOTES:
 *      1. 128 (0x8000)
 *
 */
BOOL_T IP_LIB_IsIpInClassB(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    if(ip_address[0] <= IP_CLASS_B_MAX && ip_address[0] > IP_CLASS_A_MAX)
    {
        return TRUE;
    }

    return FALSE;
}   /*  end of IP_LIB_IsIpInClassB  */


/* FUNCTION NAME : IP_LIB_IsIpInClassC
 * PURPOSE:
 *      Check the IP is in Class C or not ?
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class C; 192~223
 *      FALSE -- no, not in Class C.
 *
 * NOTES:
 *      1. 192 (0x1100)
 *
 */
BOOL_T IP_LIB_IsIpInClassC(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */

    if(ip_address[0] < IP_EXT_ADD && ip_address[0] > IP_CLASS_B_MAX)
    {
        return TRUE;
    }

    return FALSE;
}   /*  end of IP_LIB_IsIpInClassC  */


/* FUNCTION NAME : IP_LIB_IsIpInClassD
 * PURPOSE:
 *      Check the IP is in Class D or not ? (the multicast group)
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class D; 224~239
 *      FALSE -- no, not in Class D.
 *
 * NOTES:
 *      1. 224 (0x1110)
 *
 */
BOOL_T IP_LIB_IsIpInClassD(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    return (((ip_address[0] & 0xF0)==0xE0)?TRUE:FALSE);

}   /*  end of IP_LIB_IsIpInClassD  */


/* FUNCTION NAME : IP_LIB_IsIpInClassE
 * PURPOSE:
 *      Check the IP is in Class E or not ? (Testing group)
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class E; 240~255
 *      FALSE -- no, not in Class E.
 *
 * NOTES:
 *      1. 240 (0x1111)
 *
 */
BOOL_T IP_LIB_IsIpInClassE(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */
    return (((ip_address[0] & 0xF0)==0xF0)?TRUE:FALSE);

}   /*  end of IP_LIB_IsIpInClassE  */

/* FUNCTION NAME : IP_LIB_CompareIp
 * PURPOSE:
 *      Compare two UI32_T ip address, and return result.
 * INPUT:
 *      ip_address_1  -- first IP address.
*       ip_address_2  -- second IP address.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      1   -- ip_address_1 great than ip_address_2
 *      0   -- ip_address_1 equal to   ip_address_2
*       -1  -- ip_address_1 less  tahn ip_address_2
 *
 * NOTES:
 *      None.
 *
 */
int IP_LIB_CompareIp(UI8_T ip_address_1[SYS_ADPT_IPV4_ADDR_LEN], UI8_T ip_address_2[SYS_ADPT_IPV4_ADDR_LEN])
{
    return memcmp(ip_address_1, ip_address_2, SYS_ADPT_IPV4_ADDR_LEN);
}

/* FUNCTION NAME : IP_LIB_GetNetworkID
 * PURPOSE:
 *      Get network ID from ip and mask.
 *
 * INPUT:
 *      ip_address  -- the ip of subnet.
 *      ip_mask     -- mask of subnet.
 *
 * OUTPUT:
 *      network_id   -- the network ID of subnet.
 *
 * RETURN:
 *      IP_LIB_OK           -- successfully.
 *
 * NOTES:
 *      None.
 *
 */
UI32_T IP_LIB_GetNetworkID(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN], UI8_T network_id[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY
     */

    /*  network ID  */
    network_id[0] = ip_address[0] & mask[0];
    network_id[1] = ip_address[1] & mask[1];
    network_id[2] = ip_address[2] & mask[2];
    network_id[3] = ip_address[3] & mask[3];

    return IP_LIB_OK;
}

/* FUNCTION NAME : IP_LIB_IsValidNetworkMask
 * PURPOSE:
 *      Verify the IP mask is continue bit 1.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE      -- It is a valid Network mask
 *      FALSE     -- It is not a valid Network mask
 *
 * NOTES:
 *      1. true if "hole" in mask; true because x&-x always has
 *         exactly one bit set, which should be equal -x
 */
BOOL_T IP_LIB_IsValidNetworkMask(UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    UI32_T   ip_mask;
    UI32_T   val;
    /* BODY
     */
    memcpy((UI8_T *)&ip_mask, mask, SYS_ADPT_IPV4_ADDR_LEN);
    val = L_STDLIB_Ntoh32(ip_mask);

    if(((val)&(~(val)+1)) != (~(val)+1))
        return FALSE;

    return TRUE;
}

/* FUNCTION NAME : IP_LIB_IsSubnetBroadcastIp
 * PURPOSE:
 *      Is the ip is a broadcast IP for subnet ?
 *
 * INPUT:
 *      subnet_mask - mask of subnet
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a subnet broadcast IP.
 *      FALSE -- ip not a subnet broadcast IP.
 *
 * NOTES:
 *      1. It's classless checking.
 *          (ip&~mask)==(b'cast&~mask)
 */
BOOL_T IP_LIB_IsSubnetBroadcastIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    UI8_T bcast_ip[SYS_ADPT_IPV4_ADDR_LEN];
    /* BODY
     */

    IP_LIB_GetSubnetBroadcastIp(ip_address, mask, bcast_ip);

    if(0 == memcmp(ip_address, bcast_ip, SYS_ADPT_IPV4_ADDR_LEN))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* FUNCTION NAME : IP_LIB_IsHostIdZero
 * PURPOSE:
 *      Is the ip is a network IP for subnet ?
 *
 * INPUT:
 *      subnet_mask - mask of subnet
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a network-ip, can't as interface-ip.
 *      FALSE -- ip not a network IP.
 *
 * NOTES:
 *      1. It's classless checking.
 *          (ip&~mask)==(b'cast&~mask)
 */
BOOL_T IP_LIB_IsHostIdZero(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */

    if(((ip_address[0] & ~mask[0]) == 0) &&
       ((ip_address[1] & ~mask[1]) == 0) &&
       ((ip_address[2] & ~mask[2]) == 0) &&
       ((ip_address[3] & ~mask[3]) == 0)
      )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/* FUNCTION NAME : IP_LIB_IsIpMaskZero
 * PURPOSE:
 *      Is the ip&network mask is zero ?
 *
 * INPUT:
 *      subnet_mask - mask of subnet
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *
 */
BOOL_T IP_LIB_IsIpMaskZero(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY
     */

    if(((ip_address[0] & mask[0]) == 0) &&
       ((ip_address[1] & mask[1]) == 0) &&
       ((ip_address[2] & mask[2]) == 0) &&
       ((ip_address[3] & mask[3]) == 0)
      )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/* FUNCTION NAME : IP_LIB_IsIpBelongToSubnet
 * PURPOSE:
 *      Verify the IP is belong to the subnet.
 *
 * INPUT:
 *      subnet_ip
 *      subnet_mask_len
 *      ip_addr             -- address under verification
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE -- yes, ip_addr belong to subnet.
 *      FALSE-- no, ip_addr is another subnet.
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIpBelongToSubnet(const UI8_T* subnet_ip,
                                UI32_T subnet_mask_len,
                                UI8_T* ip_address)
{
    /* reference L_PREFIX_Match() */

    /* LOCAL CONSTANT DECLARATIONS
     */
    static UI8_T maskbit[] =
    {
        0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
    };

    /* LOCAL VARIABLES DEFINITION
     */
    UI32_T offset, shift;
    /* BODY
     */

    offset = subnet_mask_len/ 8;
    shift = subnet_mask_len % 8;

    if (shift)
        if (maskbit[shift] & (subnet_ip[offset] ^ ip_address[offset]))
            return FALSE;

    while (offset--)
        if (subnet_ip[offset] != ip_address[offset])
            return FALSE;

        return TRUE;

}

/* FUNCTION NAME : IP_LIB_UI32toByteArray
 * PURPOSE:
 *      Convert IP address from UI32_T type to UI8_T[4].
 *
 * INPUT:
 *      in                  -- IP address in UI32_T format.
 *
 * OUTPUT:
 *      out                 -- IP address in UI8_T[4] format.
 *
 * RETURN:
 *      IP_LIB_OK           -- Converted successfully.
 *      IP_LIB_INVALID_ARG  -- Invalid IP address inputed.
 *
 * NOTES:
 *      None.
 */
UI32_T IP_LIB_UI32toArray(UI32_T in, UI8_T out[SYS_ADPT_IPV4_ADDR_LEN])
{
    memcpy(out, &in, sizeof(UI32_T));
    return IP_LIB_OK;
}

/* FUNCTION NAME : IP_LIB_ArraytoUI32
 * PURPOSE:
 *      Convert IP address from UI8_T[4] to UI32_T type in network order.
 * INPUT:
 *      byte_ip             -- IP address in UI8_T[4] format.
 * OUTPUT:
 *      ui32_ip             -- IP address in UI32_T format (network order).
 * RETURN:
 *      IP_LIB_OK           -- Converted successfully.
 *      IP_LIB_INVALID_ARG  -- Invalid IP address inputed.
 * NOTES:
 *      None.
 */
UI32_T IP_LIB_ArraytoUI32(UI8_T byte_ip[SYS_ADPT_IPV4_ADDR_LEN], UI32_T *ui32_ip)
{
    memcpy(ui32_ip, byte_ip, sizeof(UI32_T));
    return IP_LIB_OK;
}


/* FUNCTION NAME : IP_LIB_IsIpInterfaceUp
 * PURPOSE:
 *      Is the l3 interface is up or administrative down ?
 *
 * INPUT:
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the l3 interface is UP.
 *      FALSE -- the l3 interface is administrative DOWN.
 *
 * NOTES:
 */
BOOL_T IP_LIB_IsIpInterfaceUp(UI16_T flags)
{
    if ((flags & IFF_UP) == IFF_UP)
        return TRUE;

    return FALSE;
}


/* FUNCTION NAME : IP_LIB_IsIpInterfaceRunning
 * PURPOSE:
 *      Is the l3 interface is Running ?
 *
 * INPUT:
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the l3 interface is RUNNING.
 *      FALSE -- the l3 interface is DOWN.
 *
 * NOTES:
 */
BOOL_T IP_LIB_IsIpInterfaceRunning(UI16_T flags)
{
    if ((flags & IFF_RUNNING) == IFF_RUNNING)
        return TRUE;

    return FALSE;
}


#if 0
rfc 4291,
   The type of an IPv6 address is identified by the high-order bits of
   the address, as follows:

      Address type         Binary prefix        IPv6 notation   Section
      ------------         -------------        -------------   -------
      Unspecified          00...0  (128 bits)   ::/128          2.5.2
      Loopback             00...1  (128 bits)   ::1/128         2.5.3
      Multicast            11111111             FF00::/8        2.7
      Link-Local unicast   1111111010           FE80::/10       2.5.6
      Global Unicast       (everything else)

#endif

/* FUNCTION NAME : IP_LIB_IsIPv6UnspecifiedAddr
 * PURPOSE:
 *      Check if this ipv6 address is unspecified address (::).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIPv6UnspecifiedAddr(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    const static UI8_T ipv6_zero_addr[SYS_ADPT_IPV6_ADDR_LEN] = {}; /* initial to all 0 */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY
     */
    return (!memcmp(addr, ipv6_zero_addr, SYS_ADPT_IPV6_ADDR_LEN));
}

/* FUNCTION NAME : IP_LIB_IsIPv6LoopbackAddr
 * PURPOSE:
 *      Check if this ipv6 address is loopback address (::1).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIPv6LoopbackAddr(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    const static UI8_T ipv6_loopback_addr[SYS_ADPT_IPV6_ADDR_LEN] = \
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}; /* not {1} */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY
     */
    return (!memcmp(addr, ipv6_loopback_addr, SYS_ADPT_IPV6_ADDR_LEN));
}

/* FUNCTION NAME : IP_LIB_IsIPv6LinkLocal
 * PURPOSE:
 *      Check if this ipv6 address is link local address (FE80::/10).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE 
 *      FALSE
 *
 * NOTES:
 *      None. 
 */
BOOL_T IP_LIB_IsIPv6LinkLocal(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN])
{

    /* LOCAL CONSTANT DECLARATIONS
     */
    const static UI8_T ipv6_link_local_prefix[2] = {0xFE,0x80};
    const static UI8_T ipv6_prefix_mask[2] = {0xFF,0xC0};
    /* LOCAL VARIABLES DEFINITION
     */
    UI8_T addr_prefix[2]={0};
    /* BODY
     */
    addr_prefix[0] = addr[0]&ipv6_prefix_mask[0];
    addr_prefix[1] = addr[1]&ipv6_prefix_mask[1]; 
    return (!memcmp(addr_prefix, ipv6_link_local_prefix, 2));

}

/* FUNCTION NAME : IP_LIB_IsIPv6Multicast
 * PURPOSE:
 *      Check if this ipv6 address is multicast address (FF00::/8).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIPv6Multicast(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    const static UI8_T ipv6_multicast_prefix[1] = {0xFF};

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY
     */
    return (!memcmp(addr, ipv6_multicast_prefix, 1));
}

/* FUNCTION NAME : IP_LIB_IsIPv6SolicitedMulticast
 * PURPOSE:
 *      Check if this ipv6 address is solicited multicast address (FF02::1:FF/104).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE 
 *      FALSE
 *
 * NOTES:
 *      None. 
 */
BOOL_T IP_LIB_IsIPv6SolicitedMulticast(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    const static UI8_T ipv6_solicited_multicast[SYS_ADPT_IPV6_ADDR_LEN] = 
        {0xFF,0x02,0,0,0,0,0,0,0,0,0,0x01,0xFF,0,0,0};

    /* LOCAL VARIABLES DEFINITION
     */
    
    /* BODY
     */
    return (!memcmp(addr, ipv6_solicited_multicast, 13));
}

/* FUNCTION NAME : IP_LIB_IsIPv6LinkLocalAllNodeMulticast
 * PURPOSE:
 *      Check if this ipv6 address is link local scope all node multicast address (FF02::1).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE 
 *      FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIPv6LinkLocalAllNodeMulticast(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN])
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    const static UI8_T ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN] = 
    {0xFF,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};

    /* LOCAL VARIABLES DEFINITION
     */
    
    /* BODY
     */
    return (!memcmp(addr, ipv6_addr, SYS_ADPT_IPV6_ADDR_LEN));
}

/* FUNCTION NAME : IP_LIB_CheckIPv6PrefixForInterface
 * PURPOSE:
 *      Check if this ipv6 address is good for the interface while address configuration.
 *
 * INPUT:
 *      addr    -- ipv6 address
 *      preflen -- prefix length
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None.
 */
UI32_T IP_LIB_CheckIPv6PrefixForInterface(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN], UI32_T prefix_len)
{

    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */


    /* BODY
     */
    /* check ipv6 address and mask,
     1. Unspecified     ::/128
     2. Loopback        ::1/128
     3. Multicast       FF00::/8
     */
    if (TRUE == IP_LIB_IsIPv6UnspecifiedAddr(addr))
        return IP_LIB_INVALID_IPV6_UNSPECIFIED;

    if (TRUE == IP_LIB_IsIPv6LoopbackAddr(addr))
        return IP_LIB_INVALID_IPV6_LOOPBACK;

    if (TRUE == IP_LIB_IsIPv6Multicast(addr))
        return IP_LIB_INVALID_IPV6_MULTICAST;

    return IP_LIB_OK;
}

/* FUNCTION NAME : IP_LIB_GetPrefixAddr
 * PURPOSE:
 *      Mask the addr with prefix length to get the prefix address
 *
 * INPUT:
 *      addr        -- addr
 *      addr_len    -- size is 4 or 16
 *      preflen     -- prefix length
 *
 * OUTPUT:
 *      addr_out    -- the prefix address
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      1. For IPv4/IPv6.
 *      2. The prefix length assume the same as INPUT: preflen.
 *      3. Bit copy is used if (preflen%8)
 */
BOOL_T IP_LIB_GetPrefixAddr(UI8_T addr[], UI16_T addr_len, UI32_T preflen, UI8_T addr_out[])
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    int num_byte, num_bit;

    /* BODY
     */
    if((addr_len*8) < preflen)
        return FALSE;

    num_byte = preflen / 8;
    num_bit = preflen % 8;

    /* clear output address */
    memset(addr_out, 0, addr_len);

    /* copy byte part */
    memcpy(addr_out, addr, num_byte);

    /* copy bit part */
    if (num_bit != 0) /* copy the bits part */
    {
        addr_out[num_byte] |= addr[num_byte] & (0xFF<<(8-num_bit));
    }

    return TRUE;
}

/* FUNCTION NAME : IP_LIB_IsLoopbackInterface
 * PURPOSE:
 *      Is the interface is Loopback ?
 *
 * INPUT:
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the interface is Loopback.
 *      FALSE -- the interface isn't Loopback.
 *
 * NOTES:
 */
BOOL_T IP_LIB_IsLoopbackInterface(UI16_T flags)
{
    if ((flags & IFF_LOOPBACK) == IFF_LOOPBACK)
        return TRUE;

    return FALSE;
}

/* FUNCTION NAME : IP_LIB_ConvertLoopbackIdToIfindex
 * PURPOSE:
 *      This function will convert a loopback ID to a ifindex.
 *
 * INPUT:
 *      lo_id
 *
 * OUTPUT:
 *      ifindex
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T  IP_LIB_ConvertLoopbackIdToIfindex(UI32_T lo_id, UI32_T *ifindex)
{
    /* BODY */

    if (lo_id >= SYS_ADPT_MAX_NBR_OF_LOOPBACK_IF)
    {
        *ifindex = 0;
        return  FALSE;
    }

    *ifindex = lo_id + SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER;
    return  TRUE;
}


/* FUNCTION NAME : IP_LIB_ConvertLoopbackIfindexToId
 * PURPOSE:
 *      This function will convert a ifindex to a Loopback ID.
 *
 * INPUT:
 *      ifindex
 *
 * OUTPUT:
 *      lo_id
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T  IP_LIB_ConvertLoopbackIfindexToId(UI32_T ifindex, UI32_T *lo_id)
{
    /* BODY */

    if (ifindex < SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER ||
        ifindex >= SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER + SYS_ADPT_MAX_NBR_OF_LOOPBACK_IF)
    {
        *lo_id = 0;
        return  FALSE;
    }

    *lo_id = ifindex - SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER;

    return TRUE;
}


/* FUNCTION NAME : IP_LIB_IsIpPrefixEqual
 * PURPOSE:
 *      Check where the ip prefix is equal
 *
 * INPUT:
 *      ip_addr1_p  -- ip address 1
 *      ip_addr2_p  -- ip address 2
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE   -- the prefix of ip addr 1 and ip addr 2 are equal
 *      FALSE  -- otherwise
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIpPrefixEqual(L_INET_AddrIp_T *ip_addr1_p, L_INET_AddrIp_T *ip_addr2_p)
{
    UI8_T addr_out1[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T addr_out2[SYS_ADPT_IPV6_ADDR_LEN];
    UI32_T compare_len;

    if (ip_addr1_p == NULL || ip_addr2_p == NULL)
        return FALSE;

    if (ip_addr1_p->preflen != ip_addr2_p->preflen)
        return FALSE;

    compare_len = (ip_addr1_p->preflen+7)/8;

    if (!IP_LIB_GetPrefixAddr(ip_addr1_p->addr, sizeof(ip_addr1_p->addr), ip_addr1_p->preflen, addr_out1))
        return FALSE;

    if (!IP_LIB_GetPrefixAddr(ip_addr2_p->addr, sizeof(ip_addr2_p->addr), ip_addr2_p->preflen, addr_out2))
        return FALSE;

    return (0 == memcmp(addr_out1, addr_out2, compare_len));
}

/* FUNCTION NAME : IP_LIB_IsValidSocketPortForServerListen
 * PURPOSE:
 *      To verify if specified IP socket port and type is valid for
 *          specified APP ID to use as server's listening port
 *
 * INPUT:
 *      chk_skt_port - port number for IP socket to check (host order)
 *      chk_skt_type - socket type for IP socket to check
 *                                     (refer to IP_LIB_SktType_E)
 *      chk_app_id   - APP ID to check (refer to IP_LIB_ChkAppId_E)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- It is a valid socket port for specified APP ID
 *      FALSE -- It is not a valid socket port for specified APP ID
 *
 * NOTES:
 *      1. checking for local server's listening port
 */
BOOL_T IP_LIB_IsValidSocketPortForServerListen(UI16_T chk_skt_port, UI16_T chk_skt_type, UI32_T chk_app_id)
{
    static IP_LIB_SkPortInfo_T  sk_port_db[] = {
                IP_LIB_CHKAPP_SKPORT_LST(IP_LIB_CHKAPP_APPSKPT_INFO) };
    IP_LIB_SkPortInfo_T *found_p;
    BOOL_T              ret = TRUE;

    found_p = (IP_LIB_SkPortInfo_T *) bsearch (
                    &chk_skt_port,
                    sk_port_db,
                    sizeof(sk_port_db) / sizeof (sk_port_db[0]),
                    sizeof (sk_port_db[0]),
                    &IP_LIB_LocalCmpChkAppScoketPortCmp);

    if (NULL != found_p)
    {
        /* if chk_app_id is not allowed to use this port
         *    if chk_skt_type is used by allow_app
         *       return FALSE
         */
        if (chk_app_id != found_p->allow_app)
        {
            if (found_p->skt_type_bmp & (1 << chk_skt_type))
            {
                ret = FALSE;
            }
        }
    }

    return ret;
}

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T IP_LIB_ConvertVfiIdToIfindex(UI32_T vfi_id, UI32_T *ifindex_p)
{
    if (vfi_id < SYS_ADPT_MIN_VXLAN_VFI_ID || vfi_id > SYS_ADPT_MAX_VXLAN_VFI_ID)
    {
        return FALSE;
    }

    *ifindex_p = vfi_id + SYS_ADPT_VXLAN_FIRST_IF_INDEX_NUMBER - SYS_ADPT_MIN_VXLAN_VFI_ID;
    return  TRUE;
}

BOOL_T IP_LIB_ConvertVfiIdFromIfindex(UI32_T ifindex, UI32_T *vfi_id_p)
{
    if (!IS_VXLAN_IFINDEX(ifindex))
    {
        return  FALSE;
    }

    *vfi_id_p = ifindex - SYS_ADPT_VXLAN_FIRST_IF_INDEX_NUMBER + SYS_ADPT_MIN_VXLAN_VFI_ID;
    return  TRUE;
}
#endif

/*==========================
 * LOCAL SUBPROGRAM BODIES
 *==========================
 */
static int IP_LIB_LocalCmpChkAppScoketPortCmp (const void *key_p, const void *member_p)
{
    IP_LIB_SkPortInfo_T     *chk_info_p;
    UI16_T                  src_skt_port;

    chk_info_p   = (IP_LIB_SkPortInfo_T *) member_p;
    src_skt_port = *((UI16_T *) key_p);

    return (src_skt_port - chk_info_p->skt_port);
}

