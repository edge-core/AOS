/******************************************************************************
 ******************************************************************************
 **** This file was automatically generated by Epilogue Technology's
 **** Emissary SNMP MIB Compiler, version 6.3.
 **** This file was generated using the -leaf switch.
 **** 
 **** This file #defines C preprocessor macros providing a variety of
 **** information for the leaf objects in the MIB.
 **** 
 **** The file includes a LEAF_xxx macro for each leaf object in the
 **** MIB (xxx is replaced by the object's name).  The value of the
 **** LEAF_xxx macro is the final component of the object's object
 **** identifier.
 **** 
 **** If the object's SYNTAX clause included named INTEGER values,
 **** then there is a VAL_xxx_yyy macro for each named value (xxx is
 **** replaced by the object's name and yyy by the value's name).  The
 **** value of the VAL_xxx_yyy macro is the value associated with the
 **** named value.
 **** 
 **** If the object's SYNTAX clause specified a set of range limitations
 **** for the value of the object, then there are one or more sets of
 **** MIN_xxx and MAX_xxx macros specifying the lower and upper bound of
 **** each range limitation.
 **** 
 **** If the object's SYNTAX clause specified a set of size constraints
 **** for the value of the object, then there are one or more sets of
 **** MINSIZE_xxx and MAXSIZE_xxx macros specifying the lower and upper
 **** bound of each size constraint.  (If the size constraint is a single
 **** value rather than a range then the MINSIZE_xxx and MAXSIZE_xxx
 **** macros are replaced by a single SIZE_xxx macro.)
 **** 
 **** DO NOT MODIFY THIS FILE BY HAND.
 **** 
 **** Last build date: Fri Nov  9 15:21:45 2001
 **** from files:
 ****  Rfc1213.mib, Rfc1447-SNMPv2-PARTY.mib, Rfc1450-SNMPv2.mib,
 ****  Rfc1573-IANAifType.mib, Rfc2863-IF.mib, Rfc2863.ctl
 ******************************************************************************
 ******************************************************************************
 */
#ifndef  LEAF_2863_H
#define  LEAF_2863_H

#define LEAF_ifNumber	1
#define LEAF_ifIndex	1
#define MIN_ifIndex	1L
#define MAX_ifIndex	2147483647L
#define LEAF_ifDescr	2
#define MINSIZE_ifDescr	0L
#define MAXSIZE_ifDescr	255L
#define LEAF_ifType	3
#define VAL_ifType_other	1L
#define VAL_ifType_regular1822	2L
#define VAL_ifType_hdh1822	3L
#define VAL_ifType_ddnX25	4L
#define VAL_ifType_rfc877x25	5L
#define VAL_ifType_ethernetCsmacd	6L
#define VAL_ifType_iso88023Csmacd	7L
#define VAL_ifType_iso88024TokenBus	8L
#define VAL_ifType_iso88025TokenRing	9L
#define VAL_ifType_iso88026Man	10L
#define VAL_ifType_starLan	11L
#define VAL_ifType_proteon10Mbit	12L
#define VAL_ifType_proteon80Mbit	13L
#define VAL_ifType_hyperchannel	14L
#define VAL_ifType_fddi	15L
#define VAL_ifType_lapb	16L
#define VAL_ifType_sdlc	17L
#define VAL_ifType_ds1	18L
#define VAL_ifType_e1	19L
#define VAL_ifType_basicISDN	20L
#define VAL_ifType_primaryISDN	21L
#define VAL_ifType_propPointToPointSerial	22L
#define VAL_ifType_ppp	23L
#define VAL_ifType_softwareLoopback	24L
#define VAL_ifType_eon	25L
#define VAL_ifType_ethernet3Mbit	26L
#define VAL_ifType_nsip	27L
#define VAL_ifType_slip	28L
#define VAL_ifType_ultra	29L
#define VAL_ifType_ds3	30L
#define VAL_ifType_sip	31L
#define VAL_ifType_frameRelay	32L
#define VAL_ifType_rs232	33L
#define VAL_ifType_para	34L
#define VAL_ifType_arcnet	35L
#define VAL_ifType_arcnetPlus	36L
#define VAL_ifType_atm	37L
#define VAL_ifType_miox25	38L
#define VAL_ifType_sonet	39L
#define VAL_ifType_x25ple	40L
#define VAL_ifType_iso88022llc	41L
#define VAL_ifType_localTalk	42L
#define VAL_ifType_smdsDxi	43L
#define VAL_ifType_frameRelayService	44L
#define VAL_ifType_v35	45L
#define VAL_ifType_hssi	46L
#define VAL_ifType_hippi	47L
#define VAL_ifType_modem	48L
#define VAL_ifType_aal5	49L
#define VAL_ifType_sonetPath	50L
#define VAL_ifType_sonetVT	51L
#define VAL_ifType_smdsIcip	52L
#define VAL_ifType_propVirtual	53L
#define VAL_ifType_propMultiplexor	54L
#define LEAF_ifMtu	4
#define LEAF_ifSpeed	5
#define LEAF_ifPhysAddress	6
#define LEAF_ifAdminStatus	7
#define VAL_ifAdminStatus_up	1L
#define VAL_ifAdminStatus_down	2L
#define VAL_ifAdminStatus_testing	3L
#define LEAF_ifOperStatus	8
#define VAL_ifOperStatus_up	1L
#define VAL_ifOperStatus_down	2L
#define VAL_ifOperStatus_testing	3L
#define VAL_ifOperStatus_unknown	4L
#define VAL_ifOperStatus_dormant	5L
#define VAL_ifOperStatus_notPresent	6L
#define VAL_ifOperStatus_lowerLayerDown	7L
#define LEAF_ifLastChange	9
#define LEAF_ifInOctets	10
#define LEAF_ifInUcastPkts	11
#define LEAF_ifInNUcastPkts	12
#define LEAF_ifInDiscards	13
#define LEAF_ifInErrors	14
#define LEAF_ifInUnknownProtos	15
#define LEAF_ifOutOctets	16
#define LEAF_ifOutUcastPkts	17
#define LEAF_ifOutNUcastPkts	18
#define LEAF_ifOutDiscards	19
#define LEAF_ifOutErrors	20
#define LEAF_ifOutQLen	21
#define LEAF_ifSpecific	22
#define LEAF_ifName	1
#define MINSIZE_ifName	0L
#define MAXSIZE_ifName	255L
#define LEAF_ifInMulticastPkts	2
#define LEAF_ifInBroadcastPkts	3
#define LEAF_ifOutMulticastPkts	4
#define LEAF_ifOutBroadcastPkts	5
#define LEAF_ifHCInOctets	6
#define LEAF_ifHCInUcastPkts	7
#define LEAF_ifHCInMulticastPkts	8
#define LEAF_ifHCInBroadcastPkts	9
#define LEAF_ifHCOutOctets	10
#define LEAF_ifHCOutUcastPkts	11
#define LEAF_ifHCOutMulticastPkts	12
#define LEAF_ifHCOutBroadcastPkts	13
#define LEAF_ifLinkUpDownTrapEnable	14
#define VAL_ifLinkUpDownTrapEnable_enabled	1L
#define VAL_ifLinkUpDownTrapEnable_disabled	2L
#define LEAF_ifHighSpeed	15
#define LEAF_ifPromiscuousMode	16
#define VAL_ifPromiscuousMode_true	1L
#define VAL_ifPromiscuousMode_false	2L
#define LEAF_ifConnectorPresent	17
#define VAL_ifConnectorPresent_true	1L
#define VAL_ifConnectorPresent_false	2L
#define LEAF_ifAlias	18
#define MINSIZE_ifAlias	0L
#define MAXSIZE_ifAlias	64L
#define LEAF_ifCounterDiscontinuityTime	19
#define LEAF_ifStackHigherLayer	1
#define MIN_ifStackHigherLayer	0L
#define MAX_ifStackHigherLayer	2147483647L
#define LEAF_ifStackLowerLayer	2
#define MIN_ifStackLowerLayer	0L
#define MAX_ifStackLowerLayer	2147483647L
#define LEAF_ifStackStatus	3
#define VAL_ifStackStatus_active	1L
#define VAL_ifStackStatus_notInService	2L
#define VAL_ifStackStatus_notReady	3L
#define VAL_ifStackStatus_createAndGo	4L
#define VAL_ifStackStatus_createAndWait	5L
#define VAL_ifStackStatus_destroy	6L
#define LEAF_ifTestId	1
#define MIN_ifTestId	0L
#define MAX_ifTestId	2147483647L
#define LEAF_ifTestStatus	2
#define VAL_ifTestStatus_notInUse	1L
#define VAL_ifTestStatus_inUse	2L
#define LEAF_ifTestType	3
#define LEAF_ifTestResult	4
#define VAL_ifTestResult_none	1L
#define VAL_ifTestResult_success	2L
#define VAL_ifTestResult_inProgress	3L
#define VAL_ifTestResult_notSupported	4L
#define VAL_ifTestResult_unAbleToRun	5L
#define VAL_ifTestResult_aborted	6L
#define VAL_ifTestResult_failed	7L
#define LEAF_ifTestCode	5
#define LEAF_ifTestOwner	6
#define MINSIZE_ifTestOwner	0L
#define MAXSIZE_ifTestOwner	255L
#define LEAF_ifRcvAddressAddress	1
#define LEAF_ifRcvAddressStatus	2
#define VAL_ifRcvAddressStatus_active	1L
#define VAL_ifRcvAddressStatus_notInService	2L
#define VAL_ifRcvAddressStatus_notReady	3L
#define VAL_ifRcvAddressStatus_createAndGo	4L
#define VAL_ifRcvAddressStatus_createAndWait	5L
#define VAL_ifRcvAddressStatus_destroy	6L
#define LEAF_ifRcvAddressType	3
#define VAL_ifRcvAddressType_other	1L
#define VAL_ifRcvAddressType_volatile	2L
#define VAL_ifRcvAddressType_nonVolatile	3L
#define LEAF_ifTableLastChange	5
#define LEAF_ifStackLastChange	6

#endif /* end of #ifndef  LEAF_2863_H */
