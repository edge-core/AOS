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
 **** Last build date: Thu Jun 19 13:36:47 2003
 **** from files:
 ****  ..\MibFiles\SnmpFramework_Rfc3411.mib, ctl\SnmpFramework_Rfc3411.ctl,
 ****  ..\MibFiles\SnmpTarget_Rfc3413.mib
 ******************************************************************************
 ******************************************************************************
 */
#ifndef  LEAF_3413T_H
#define  LEAF_3413T_H

#define LEAF_snmpTargetSpinLock	1
#define MIN_snmpTargetSpinLock	0L
#define MAX_snmpTargetSpinLock	2147483647L
#define LEAF_snmpTargetAddrName	1
#define MINSIZE_snmpTargetAddrName	1L
#define MAXSIZE_snmpTargetAddrName	64L /*LEO: follow CISCO 09/03/2004 */ /* stardard MIB is 32, needs further discussion for Leaf_3413N/T.h */
#define LEAF_snmpTargetAddrTDomain	2
#define LEAF_snmpTargetAddrTAddress	3
#define MINSIZE_snmpTargetAddrTAddress	1L
#define MAXSIZE_snmpTargetAddrTAddress	255L
#define LEAF_snmpTargetAddrTimeout	4
#define MIN_snmpTargetAddrTimeout	0L
#define MAX_snmpTargetAddrTimeout	2147483647L
#define LEAF_snmpTargetAddrRetryCount	5
#define MIN_snmpTargetAddrRetryCount	0L
#define MAX_snmpTargetAddrRetryCount	255L
#define LEAF_snmpTargetAddrTagList	6
#define MINSIZE_snmpTargetAddrTagList	0L
#define MAXSIZE_snmpTargetAddrTagList	255L
#define LEAF_snmpTargetAddrParams	7
#define MINSIZE_snmpTargetAddrParams	1L
#define MAXSIZE_snmpTargetAddrParams	64L /*LEO: follow CISCO 09/03/2004 */ /* stardard MIB is 32, needs further discussion for Leaf_3413N/T.h */
#define LEAF_snmpTargetAddrStorageType	8
#define VAL_snmpTargetAddrStorageType_other	1L
#define VAL_snmpTargetAddrStorageType_volatile	2L
#define VAL_snmpTargetAddrStorageType_nonVolatile	3L
#define VAL_snmpTargetAddrStorageType_permanent	4L
#define VAL_snmpTargetAddrStorageType_readOnly	5L
#define LEAF_snmpTargetAddrRowStatus	9
#define VAL_snmpTargetAddrRowStatus_active	1L
#define VAL_snmpTargetAddrRowStatus_notInService	2L
#define VAL_snmpTargetAddrRowStatus_notReady	3L
#define VAL_snmpTargetAddrRowStatus_createAndGo	4L
#define VAL_snmpTargetAddrRowStatus_createAndWait	5L
#define VAL_snmpTargetAddrRowStatus_destroy	6L
#define LEAF_snmpTargetParamsName	1
#define MINSIZE_snmpTargetParamsName	1L
#define MAXSIZE_snmpTargetParamsName	64L /*LEO: follow CISCO 09/03/2004 */ /* stardard MIB is 32, needs further discussion for Leaf_3413N/T.h */
#define LEAF_snmpTargetParamsMPModel	2
#define MIN_snmpTargetParamsMPModel	0L
#define MAX_snmpTargetParamsMPModel	2147483647L
#define LEAF_snmpTargetParamsSecurityModel	3
#define MIN_snmpTargetParamsSecurityModel	1L
#define MAX_snmpTargetParamsSecurityModel	2147483647L
#define LEAF_snmpTargetParamsSecurityName	4
#define MINSIZE_snmpTargetParamsSecurityName	0L
#define MAXSIZE_snmpTargetParamsSecurityName	255L
#define LEAF_snmpTargetParamsSecurityLevel	5
#define VAL_snmpTargetParamsSecurityLevel_noAuthNoPriv	1L
#define VAL_snmpTargetParamsSecurityLevel_authNoPriv	2L
#define VAL_snmpTargetParamsSecurityLevel_authPriv	3L
#define LEAF_snmpTargetParamsStorageType	6
#define VAL_snmpTargetParamsStorageType_other	1L
#define VAL_snmpTargetParamsStorageType_volatile	2L
#define VAL_snmpTargetParamsStorageType_nonVolatile	3L
#define VAL_snmpTargetParamsStorageType_permanent	4L
#define VAL_snmpTargetParamsStorageType_readOnly	5L
#define LEAF_snmpTargetParamsRowStatus	7
#define VAL_snmpTargetParamsRowStatus_active	1L
#define VAL_snmpTargetParamsRowStatus_notInService	2L
#define VAL_snmpTargetParamsRowStatus_notReady	3L
#define VAL_snmpTargetParamsRowStatus_createAndGo	4L
#define VAL_snmpTargetParamsRowStatus_createAndWait	5L
#define VAL_snmpTargetParamsRowStatus_destroy	6L
#define LEAF_snmpUnavailableContexts	4
#define LEAF_snmpUnknownContexts	5

#endif /* end of #ifndef  LEAF_3413T_H */

