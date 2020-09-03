/*
 * Module Name: sys_dflt_xor.h
 *
 * PURPOSE: Define the exclusion rules.
 *
 * NOTES:
 *
 * History:
 *          Date        Modifier        Reason
 *          2005/08/04  Justin Jan      Create this file
 *
 * Copyright(C)      Accton Corporation, 2005
 */

#ifndef SYS_DFLT_XOR_H
#define SYS_DFLT_XOR_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"



/*
 * CONSTANT SYMBOL DEFINITIONS
 */
#define SYS_DFLT_XOR_LACP_PORT_EXCLUSION                            BIT_0
#define SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION                         BIT_1
#define SYS_DFLT_XOR_PRIVATE_PORT_EXCLUSION                         BIT_2
#define SYS_DFLT_XOR_MIRROR_TO_PORT_EXCLUSION                       BIT_3
#define SYS_DFLT_XOR_MIRRORED_PORT_EXCLUSION                        BIT_4
#define SYS_DFLT_XOR_SECURITY_PORT_EXCLUSION                        BIT_5
#define SYS_DFLT_XOR_RSPAN_MIRROR_TO_PORT_EXCLUSION                 BIT_6
#define SYS_DFLT_XOR_RSPAN_MIRRORED_PORT_EXCLUSION                  BIT_7
#define SYS_DFLT_XOR_RSPAN_UPLINK_PORT_EXCLUSION                    BIT_8
#define SYS_DFLT_XOR_MIRROR_TO_PORT_NOT_VLAN_MAC_MIRROR_EXCLUSION   BIT_9
#define SYS_DFLT_XOR_XSTP_STATUS_EXCLUSION                          BIT_11
#define SYS_DFLT_XOR_EAPS_STATUS_EXCLUSION                          BIT_12
#define SYS_DFLT_XOR_XSTP_PORT_EXCLUSION                            BIT_14
#define SYS_DFLT_XOR_EAPS_PORT_EXCLUSION                            BIT_15
#define SYS_DFLT_XOR_ERPS_PORT_EXCLUSION                            BIT_16
#define SYS_DFLT_XOR_QINQ_SERVICE_PORT_EXCLUSION                    BIT_17
#define SYS_DFLT_XOR_VLAN_XLATE_PORT_EXCLUSION                      BIT_18
#define SYS_DFLT_XOR_PFC_PORT_EXCLUSION                             BIT_19
#define SYS_DFLT_XOR_FC_PORT_EXCLUSION                              BIT_20
#define SYS_DFLT_XOR_UDLD_PORT_EXCLUSION                            BIT_21
#define SYS_DFLT_XOR_MLAG_PORT_EXCLUSION                            BIT_22
#define SYS_DFLT_XOR_QOS_PORT_EXCLUSION                             BIT_23

/* VLAN rules */
#define SYS_DFLT_XOR_ERPS_CTRL_VLAN_EXCLUSION   BIT_0

/*
 * Note: YES means the rule contain the exclusion item.
 *       NO  means the rule doesn't need to check the exclusion item.
 */
#define SYS_DFLT_XOR_ENABLE_LACP_RULE           (SYS_DFLT_XOR_SECURITY_PORT_EXCLUSION           | \
                                                 SYS_DFLT_XOR_RSPAN_MIRROR_TO_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_RSPAN_MIRRORED_PORT_EXCLUSION     | \
                                                 SYS_DFLT_XOR_UDLD_PORT_EXCLUSION               | \
                                                 SYS_DFLT_XOR_RSPAN_UPLINK_PORT_EXCLUSION       | \
                                                 SYS_DFLT_XOR_MLAG_PORT_EXCLUSION               | \
                                                 SYS_DFLT_XOR_ERPS_PORT_EXCLUSION               | \
                                                 SYS_DFLT_XOR_QOS_PORT_EXCLUSION)

#define SYS_DFLT_XOR_JOIN_TO_TRUNK_RULE         (SYS_DFLT_XOR_SECURITY_PORT_EXCLUSION           | \
                                                 SYS_DFLT_XOR_RSPAN_MIRROR_TO_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_RSPAN_MIRRORED_PORT_EXCLUSION     | \
                                                 SYS_DFLT_XOR_RSPAN_UPLINK_PORT_EXCLUSION       | \
                                                 SYS_DFLT_XOR_UDLD_PORT_EXCLUSION               | \
                                                 SYS_DFLT_XOR_ERPS_PORT_EXCLUSION               | \
                                                 SYS_DFLT_XOR_MLAG_PORT_EXCLUSION               | \
                                                 SYS_DFLT_XOR_QOS_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_PRIVATE_PORT_RULE   (SYS_DFLT_XOR_RSPAN_MIRROR_TO_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_RSPAN_MIRRORED_PORT_EXCLUSION     | \
                                                 SYS_DFLT_XOR_RSPAN_UPLINK_PORT_EXCLUSION       | \
                                                 SYS_DFLT_XOR_MLAG_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_MIRROR_TO_PORT_RULE (SYS_DFLT_XOR_RSPAN_MIRROR_TO_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_RSPAN_MIRRORED_PORT_EXCLUSION     | \
                                                 SYS_DFLT_XOR_RSPAN_UPLINK_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_MIRRORED_PORT_RULE  (SYS_DFLT_XOR_RSPAN_MIRROR_TO_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_RSPAN_MIRRORED_PORT_EXCLUSION     | \
                                                 SYS_DFLT_XOR_RSPAN_UPLINK_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_SECURITY_PORT_RULE  (SYS_DFLT_XOR_LACP_PORT_EXCLUSION       | \
                                                 SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION    | \
                                                 SYS_DFLT_XOR_RSPAN_UPLINK_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_RSPAN_MIRROR_TO_PORT_RULE   (SYS_DFLT_XOR_LACP_PORT_EXCLUSION       | \
                                                         SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION    | \
                                                         SYS_DFLT_XOR_MIRRORED_PORT_EXCLUSION   | \
                                                         SYS_DFLT_XOR_MIRROR_TO_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_RSPAN_MIRRORED_PORT_RULE    (SYS_DFLT_XOR_LACP_PORT_EXCLUSION       | \
                                                         SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION    | \
                                                         SYS_DFLT_XOR_MIRRORED_PORT_EXCLUSION   | \
                                                         SYS_DFLT_XOR_MIRROR_TO_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_RSPAN_UPLINK_PORT_RULE  (SYS_DFLT_XOR_LACP_PORT_EXCLUSION       | \
                                                     SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION    | \
                                                     SYS_DFLT_XOR_MIRRORED_PORT_EXCLUSION   | \
                                                     SYS_DFLT_XOR_MIRROR_TO_PORT_EXCLUSION  | \
                                                     SYS_DFLT_XOR_SECURITY_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_XSTP_PORT_RULE      (SYS_DFLT_XOR_EAPS_PORT_EXCLUSION   | \
                                                 SYS_DFLT_XOR_ERPS_PORT_EXCLUSION   | \
                                                 SYS_DFLT_XOR_MLAG_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_ERPS_PORT_RULE      (SYS_DFLT_XOR_LACP_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION | \
                                                 SYS_DFLT_XOR_XSTP_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_EAPS_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_EAPS_PORT_RULE      (SYS_DFLT_XOR_LACP_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION | \
                                                 SYS_DFLT_XOR_XSTP_PORT_EXCLUSION    | \
                                                 SYS_DFLT_XOR_ERPS_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_QINQ_SERVICE_PORT  0

#define SYS_DFLT_XOR_SET_TO_VLAN_XLATE_PORT    0

#define SYS_DFLT_XOR_SET_TO_PFC_PORT_RULE       (SYS_DFLT_XOR_FC_PORT_EXCLUSION)
#define SYS_DFLT_XOR_SET_TO_FC_PORT_RULE        (SYS_DFLT_XOR_PFC_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_UDLD_PORT_RULE     (SYS_DFLT_XOR_LACP_PORT_EXCLUSION    | \
                                                SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION)

#define SYS_DFLT_XOR_DELETE_VLAN_RULE          (SYS_DFLT_XOR_ERPS_CTRL_VLAN_EXCLUSION)

#define SYS_DFLT_XOR_DESTROY_TRUNK_RULE        (SYS_DFLT_XOR_ERPS_PORT_EXCLUSION    | \
                                                SYS_DFLT_XOR_MLAG_PORT_EXCLUSION)

#define SYS_DFLT_XOR_SET_TO_MLAG_PORT_RULE     (SYS_DFLT_XOR_PRIVATE_PORT_EXCLUSION | \
                                                SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION | \
                                                SYS_DFLT_XOR_LACP_PORT_EXCLUSION    | \
                                                SYS_DFLT_XOR_XSTP_PORT_EXCLUSION)

#endif /* End of SYS_DFLT_XOR_H */

