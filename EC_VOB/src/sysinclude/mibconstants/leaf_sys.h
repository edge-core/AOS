/* ------------------------------------------------------------------------
 * FILE NAME - LEAFSYS.H
 * ------------------------------------------------------------------------
 * Purpose: This package defines the naming constants for the those MIB objects
 *          defined as BITS syntax.
 * Note: 1. For MIB objects defined as BITS syntax, the naming constants automatically
 *          generated by Epilogue Emissary SNMP MIB Compiler are not applicable.
 *      	Please refer to each leafxxx.h for the generated naming constants.
 *          Each subsystem SHALL NOT refer to those naming constants.
 *       2. This package redefines a new set of naming constants (with similar naming
 *          convention) for those MIB objests defined as BITS syntax.
 *       3. The naming constants defined in this package shall be used by
 *          each subsystem to support the management function for these MIB objects.
 *       4. This package shall be reusable for all the BNBU L2/L3 switch projects .
 * ------------------------------------------------------------------------
 *
 *
 *
 *  History
 *
 *   Anderson Young     11/07/2001          new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */

#ifndef LEAFSYS_H
#define LEAFSYS_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_cvrt.h"




/* NAMING CONSTANT DECLARATIONS
 */

/* Deinfe value (hex) for the following RFC1213 MIB2 objects
 *      - "sysServices"
 *
 * Note: The naming constants defined here are used to supplement the
 *       associated naming constants defined in leaf1213.h
 *
 *          -- layer  functionality --
 *
 *          1  physical (e.g., repeaters)
 *          2  datalink/subnetwork (e.g., bridges)
 *          3  internet (e.g., IP gateways)
 *          4  end-to-end  (e.g., IP hosts)
 *          7  applications (e.g., mail relays)
 *
 *         For systems including OSI protocols, layers 5 and 6 may also be counted.
 */
#define SYS_VAL_sysServices_physical	                BIT_0
#define SYS_VAL_sysServices_datalink_or_subnetwork	    BIT_1
#define SYS_VAL_sysServices_internet	                BIT_2
#define SYS_VAL_sysServices_end_to_end	                BIT_3
#define SYS_VAL_sysServices_reserverd5	                BIT_4
#define SYS_VAL_sysServices_reserverd6	                BIT_5
#define SYS_VAL_sysServices_applications	            BIT_6


/* Deinfe value (hex) for the following RFC2021 RMON2 MIB objects
 *      - "protocolDirType"
 *      - "probeCapabilities" MIB
 *
 * Note: The naming constants defined here are used to supplement the
 *       associated naming constants defined in leaf3626a.h
 */
#define SYS_VAL_protocolDirType_extensible	                BIT_0
#define SYS_VAL_protocolDirType_addressRecognitionCapable   BIT_1

#define SYS_VAL_probeCapabilities_etherStats	            BIT_0
#define SYS_VAL_probeCapabilities_historyControl	        BIT_1
#define SYS_VAL_probeCapabilities_etherHistory	            BIT_2
#define SYS_VAL_probeCapabilities_alarm	                    BIT_3
#define SYS_VAL_probeCapabilities_hosts	                    BIT_4
#define SYS_VAL_probeCapabilities_hostTopN	                BIT_5
#define SYS_VAL_probeCapabilities_matrix	                BIT_6
#define SYS_VAL_probeCapabilities_filter	                BIT_7
#define SYS_VAL_probeCapabilities_capture	                BIT_8
#define SYS_VAL_probeCapabilities_event	                    BIT_9
#define SYS_VAL_probeCapabilities_tokenRingMLStats	        BIT_10
#define SYS_VAL_probeCapabilities_tokenRingPStats	        BIT_11
#define SYS_VAL_probeCapabilities_tokenRingMLHistory	    BIT_12
#define SYS_VAL_probeCapabilities_tokenRingPHistory	        BIT_13
#define SYS_VAL_probeCapabilities_ringStation	            BIT_14
#define SYS_VAL_probeCapabilities_ringStationOrder	        BIT_15
#define SYS_VAL_probeCapabilities_ringStationConfig	        BIT_16
#define SYS_VAL_probeCapabilities_sourceRouting	            BIT_17
#define SYS_VAL_probeCapabilities_protocolDirectory	        BIT_18
#define SYS_VAL_probeCapabilities_protocolDistribution      BIT_19
#define SYS_VAL_probeCapabilities_addressMapping	        BIT_20
#define SYS_VAL_probeCapabilities_nlHost                    BIT_21
#define SYS_VAL_probeCapabilities_nlMatrix                  BIT_22
#define SYS_VAL_probeCapabilities_alHost                    BIT_23
#define SYS_VAL_probeCapabilities_alMatrix                  BIT_24
#define SYS_VAL_probeCapabilities_usrHistory                BIT_25
#define SYS_VAL_probeCapabilities_probeConfig               BIT_26


/* Deinfe value (hex) for the following RFC2674 Extened Bridge MIB objects
 *      - "dot1dDeviceCapabilities"
 *
 * Note: The naming constants defined here are used to supplement the
 *       associated naming constants defined in leaf2674.h
 */
#define SYS_VAL_dot1dDeviceCapabilities_dot1dExtendedFilteringServices  BIT_0
#define SYS_VAL_dot1dDeviceCapabilities_dot1dTrafficClasses	            BIT_1
#define SYS_VAL_dot1dDeviceCapabilities_dot1qStaticEntryIndividualPort	BIT_2
#define SYS_VAL_dot1dDeviceCapabilities_dot1qIVLCapable	                BIT_3
#define SYS_VAL_dot1dDeviceCapabilities_dot1qSVLCapable	                BIT_4
#define SYS_VAL_dot1dDeviceCapabilities_dot1qHybridCapable	            BIT_5
#define SYS_VAL_dot1dDeviceCapabilities_dot1qConfigurablePvidTagging	BIT_6
#define SYS_VAL_dot1dDeviceCapabilities_dot1dLocalVlanCapable	        BIT_7

#define SYS_VAL_dot1dPortCapabilities_dot1qDot1qTagging	                    BIT_0
#define SYS_VAL_dot1dPortCapabilities_dot1qConfigurableAcceptableFrameTypes	BIT_1
#define SYS_VAL_dot1dPortCapabilities_dot1qIngressFiltering	                BIT_2


/* Deinfe value (hex) for the following RFC2934 PIM MIB objects
 *      - "pimIpMRouteFlags"
 *
 * Note: The naming constants defined here are used to supplement the
 *       associated naming constants defined in leaf2674.h
 */
#define SYS_VAL_pimIpMRouteFlags_rpt        BIT_0
#define SYS_VAL_pimIpMRouteFlags_spt	    BIT_1


/* Deinfe value (hex) for the following ES3626A Private MIB objects
 *      - "portCapabilities"
 *
 * Note: The naming constants defined here are used to supplement the
 *       associated naming constants defined in leaf3626a.h
 */
#define SYS_VAL_portCapabilities_portCap10half	        L_CVRT_SNMP_BIT_VALUE_32(0)
#define SYS_VAL_portCapabilities_portCap10full	        L_CVRT_SNMP_BIT_VALUE_32(1)
#define SYS_VAL_portCapabilities_portCap100half	        L_CVRT_SNMP_BIT_VALUE_32(2)
#define SYS_VAL_portCapabilities_portCap100full	        L_CVRT_SNMP_BIT_VALUE_32(3)
#define SYS_VAL_portCapabilities_portCap1000half	L_CVRT_SNMP_BIT_VALUE_32(4)
#define SYS_VAL_portCapabilities_portCap1000full	L_CVRT_SNMP_BIT_VALUE_32(5)
#define SYS_VAL_portCapabilities_portCap10gHalf         L_CVRT_SNMP_BIT_VALUE_32(6)
#define SYS_VAL_portCapabilities_portCap10gFull         L_CVRT_SNMP_BIT_VALUE_32(7)
#define SYS_VAL_portCapabilities_portCap40gHalf	        L_CVRT_SNMP_BIT_VALUE_32(8)
#define SYS_VAL_portCapabilities_portCap40gFull	        L_CVRT_SNMP_BIT_VALUE_32(9)
#define SYS_VAL_portCapabilities_portCap100gFull        L_CVRT_SNMP_BIT_VALUE_32(10)
#define SYS_VAL_portCapabilities_reserverd11	        L_CVRT_SNMP_BIT_VALUE_32(11)
#define SYS_VAL_portCapabilities_reserverd12	        L_CVRT_SNMP_BIT_VALUE_32(12)
#define SYS_VAL_portCapabilities_reserverd13	        L_CVRT_SNMP_BIT_VALUE_32(13)
#define SYS_VAL_portCapabilities_portCapSym	        L_CVRT_SNMP_BIT_VALUE_32(14)
#define SYS_VAL_portCapabilities_portCapFlowCtrl	L_CVRT_SNMP_BIT_VALUE_32(15)


/* Define status and parameter values for a RS232 port
 * Note: 1. The naming constants defined here are based on the RFC1659 RS-232 MIB.
 *       2. In L2/L3 switch product line, RFC1659 RS-232 MIB is not supported.
 *          However, the status and parameter values defined by RFC1659 RS-232 MIB
 *          is used as system-wised constant for RS-232 console port configuration.
 */
#define SYS_MIN_rs232AsyncPortBits                  5
#define SYS_MAX_rs232AsyncPortBits	                8
#define SYS_VAL_rs232AsyncPortBits_five             5
#define SYS_VAL_rs232AsyncPortBits_six	            6
#define SYS_VAL_rs232AsyncPortBits_seven            7
#define SYS_VAL_rs232AsyncPortBits_eight            8

#define SYS_VAL_rs232AsyncPortStopBits_one	        1L
#define SYS_VAL_rs232AsyncPortStopBits_two	        2L
#define SYS_VAL_rs232AsyncPortStopBits_oneAndHalf	3L
#define SYS_VAL_rs232AsyncPortStopBits_dynamic	    4L

#define SYS_VAL_rs232AsyncPortParity_none	        1L
#define SYS_VAL_rs232AsyncPortParity_odd	        2L
#define SYS_VAL_rs232AsyncPortParity_even	        3L
#define SYS_VAL_rs232AsyncPortParity_mark	        4L
#define SYS_VAL_rs232AsyncPortParity_space	        5L

#define SYS_VAL_rs232AsyncPortAutobaud_enabled	    1L
#define SYS_VAL_rs232AsyncPortAutobaud_disabled	    2L


#define SYS_VAL_COLD_START                          1   /* Cold Start */
#define SYS_VAL_LOADER_CALLED_BY_DIAG               2   /* After Diag finished, load the run time image */
#define SYS_VAL_WARM_START_FOR_RELOAD               3   /* Warm Start */
#define SYS_VAL_WARM_START_FOR_TEMP_DOWNLOAD        4   /* Warm Start for temp download */
#define SYS_VAL_COLD_START_FOR_RELOAD               5   /* Cold Start for apps reload */

#define SYS_VAL_COLDSTART_MAGIC_NUMBER              0x1a2b3c4d   /* Cold Start Magic Number */
#define SYS_VAL_WARMSTART_MAGIC_NUMBER              0xd4c3b2a1   /* Warm Start Magic Number */
#define SYS_VAL_RESTART_MAGIC_NUMBER                0xd8c7b6a5   /* Re-Start Magic Number */

/* Add the naming constant for Mercury_V2-01953 */
#define SYS_VAL_LOCAL_UNIT_ID                       0   /* pass this in as "unit ID" to mean local */

/* Deinfe value (hex) for selective QinQ
 */
#define SYS_VAL_vlanDot1qTunnelSrvAction_discard        L_CVRT_SNMP_BIT_VALUE_32(0)
#define SYS_VAL_vlanDot1qTunnelSrvAction_assignPri      L_CVRT_SNMP_BIT_VALUE_32(1)
#define SYS_VAL_vlanDot1qTunnelSrvAction_assignSvid     L_CVRT_SNMP_BIT_VALUE_32(2)
#define SYS_VAL_vlanDot1qTunnelSrvAction_assignCvid     L_CVRT_SNMP_BIT_VALUE_32(3)
#define SYS_VAL_vlanDot1qTunnelSrvAction_removeCtag     L_CVRT_SNMP_BIT_VALUE_32(4)

/* IANA-MAU-MIB: IANAifMauTypeListBits and dot3MauType
 */
#define SYS_VAL_dot3MauTypeOther                    0
#define SYS_VAL_dot3MauTypeAUI                      1
#define SYS_VAL_dot3MauType10base5                  2
#define SYS_VAL_dot3MauTypeFoirl                    3
#define SYS_VAL_dot3MauType10base2                  4
#define SYS_VAL_dot3MauType10baseT                  5
#define SYS_VAL_dot3MauType10baseFP                 6
#define SYS_VAL_dot3MauType10baseFB                 7
#define SYS_VAL_dot3MauType10baseFL                 8
#define SYS_VAL_dot3MauType10broad36                9
#define SYS_VAL_dot3MauType10baseTHD                10
#define SYS_VAL_dot3MauType10baseTFD                11
#define SYS_VAL_dot3MauType10baseFLHD               12
#define SYS_VAL_dot3MauType10baseFLFD               13
#define SYS_VAL_dot3MauType100baseT4                14
#define SYS_VAL_dot3MauType100baseTXHD              15
#define SYS_VAL_dot3MauType100baseTXFD              16
#define SYS_VAL_dot3MauType100baseFXHD              17
#define SYS_VAL_dot3MauType100baseFXFD              18
#define SYS_VAL_dot3MauType100baseT2HD              19
#define SYS_VAL_dot3MauType100baseT2FD              20
#define SYS_VAL_dot3MauType1000baseXHD              21
#define SYS_VAL_dot3MauType1000baseXFD              22
#define SYS_VAL_dot3MauType1000baseLXHD             23
#define SYS_VAL_dot3MauType1000baseLXFD             24
#define SYS_VAL_dot3MauType1000baseSXHD             25
#define SYS_VAL_dot3MauType1000baseSXFD             26
#define SYS_VAL_dot3MauType1000baseCXHD             27
#define SYS_VAL_dot3MauType1000baseCXFD             28
#define SYS_VAL_dot3MauType1000baseTHD              29
#define SYS_VAL_dot3MauType1000baseTFD              30
#define SYS_VAL_dot3MauType10GbaseX                 31
#define SYS_VAL_dot3MauType10GbaseLX4               32
#define SYS_VAL_dot3MauType10GbaseR                 33
#define SYS_VAL_dot3MauType10GbaseER                34
#define SYS_VAL_dot3MauType10GbaseLR                35
#define SYS_VAL_dot3MauType10GbaseSR                36
#define SYS_VAL_dot3MauType10GbaseW                 37
#define SYS_VAL_dot3MauType10GbaseEW                38
#define SYS_VAL_dot3MauType10GbaseLW                39
#define SYS_VAL_dot3MauType10GbaseSW                40
#define SYS_VAL_dot3MauType10GbaseCX4               41
#define SYS_VAL_dot3MauType2BaseTL                  42
#define SYS_VAL_dot3MauType10PassTS                 43
#define SYS_VAL_dot3MauType100BaseBX10D             44
#define SYS_VAL_dot3MauType100BaseBX10U             45
#define SYS_VAL_dot3MauType100BaseLX10              46
#define SYS_VAL_dot3MauType1000BaseBX10D            47
#define SYS_VAL_dot3MauType1000BaseBX10U            48
#define SYS_VAL_dot3MauType1000BaseLX10             49
#define SYS_VAL_dot3MauType1000BasePX10D            50
#define SYS_VAL_dot3MauType1000BasePX10U            51
#define SYS_VAL_dot3MauType1000BasePX20D            52
#define SYS_VAL_dot3MauType1000BasePX20U            53
#define SYS_VAL_dot3MauType10GbaseT                 54
#define SYS_VAL_dot3MauType10GbaseLRM               55
#define SYS_VAL_dot3MauType1000baseKX               56
#define SYS_VAL_dot3MauType10GbaseKX4               57
#define SYS_VAL_dot3MauType10GbaseKR                58
#define SYS_VAL_dot3MauType10G1GbasePRXD1           59
#define SYS_VAL_dot3MauType10G1GbasePRXD2           60
#define SYS_VAL_dot3MauType10G1GbasePRXD3           61
#define SYS_VAL_dot3MauType10G1GbasePRXU1           62
#define SYS_VAL_dot3MauType10G1GbasePRXU2           63
#define SYS_VAL_dot3MauType10G1GbasePRXU3           64
#define SYS_VAL_dot3MauType10GbasePRD1              65
#define SYS_VAL_dot3MauType10GbasePRD2              66
#define SYS_VAL_dot3MauType10GbasePRD3              67
#define SYS_VAL_dot3MauType10GbasePRU1              68
#define SYS_VAL_dot3MauType10GbasePRU3              69
#define SYS_VAL_dot3MauType40GbaseKR4               70
#define SYS_VAL_dot3MauType40GbaseCR4               71
#define SYS_VAL_dot3MauType40GbaseSR4               72
#define SYS_VAL_dot3MauType40GbaseFR                73
#define SYS_VAL_dot3MauType40GbaseLR4               74
#define SYS_VAL_dot3MauType100GbaseCR10             75
#define SYS_VAL_dot3MauType100GbaseSR10             76
#define SYS_VAL_dot3MauType100GbaseLR4              77
#define SYS_VAL_dot3MauType100GbaseER4              78

#endif /* End of LEAFSYS_H */
