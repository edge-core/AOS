/*-----------------------------------------------------------------------------
 * FILE NAME: DHCP_TXRXRX_TYPE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Defines constants for Cmnlib to use when allocating dedicated memory pool
 *    for DHCP to send packets.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/08/10     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef DHCP_TXRX_TYPE_H
#define DHCP_TXRX_TYPE_H


/* INCLUDE FILE DECLARATIONS
 */


/* NAMING CONSTANT DECLARATIONS
 */

#define DHCP_TXRX_TYPE_TX_DEDICATED_BUFFER_POOL_PARTITION_NUM   8

#define DHCP_TXRX_TYPE_FRAME_HEADER_LEN     22
#define DHCP_TXRX_TYPE_IP_HEADER_LEN        20
#define DHCP_TXRX_TYPE_UDP_HEADER_LEN       8
#define DHCP_TXRX_TYPE_DHCP_PACKET_LEN      576

#define DHCP_TXRX_TYPE_TX_DEDICATED_BUFFER_POOL_PARTITION_SIZE  \
            (DHCP_TXRX_TYPE_FRAME_HEADER_LEN +     \
             DHCP_TXRX_TYPE_IP_HEADER_LEN +        \
             DHCP_TXRX_TYPE_UDP_HEADER_LEN +       \
             DHCP_TXRX_TYPE_DHCP_PACKET_LEN)

enum
{
    DHCP_TXRX_TRACE_ID_SENDPKTTHRUIML,
    DHCP_TXRX_TRACE_ID_SENDPKTTHRUIMLTOCLIENTPORT,
};


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


#endif /* #ifndef DHCP_TXRX_TYPE_H */
