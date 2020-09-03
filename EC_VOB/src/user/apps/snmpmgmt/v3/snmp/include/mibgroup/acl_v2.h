

#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "snmp_mgr.h"

/* function declarations */

void            init_aclMgt(void);
Netsnmp_Node_Handler do_aclAttachCtlIndex;
Netsnmp_Node_Handler do_aclAttachCtlAceType;
Netsnmp_Node_Handler do_aclAttachCtlAceIndex;
Netsnmp_Node_Handler do_aclAttachCtlAction;
/*aclIPAceTable*/
void            init_aclIpAceTable(void);
FindVarMethod   var_aclIpAceTable;
WriteMethod     write_aclIpAceSourceIpAddr;
WriteMethod     write_aclIpAceSourceIpAddrBitmask;
WriteMethod     write_aclIpAceDestIpAddr;
WriteMethod     write_aclIpAceDestIpAddrBitmask;
WriteMethod     write_aclIpAceProtocol;
WriteMethod     write_aclIpAcePrec;
WriteMethod     write_aclIpAceTos;
WriteMethod     write_aclIpAceDscp;
WriteMethod     write_aclIpAceSourcePortOp;
WriteMethod     write_aclIpAceMinSourcePort;
WriteMethod     write_aclIpAceMaxSourcePort;
WriteMethod     write_aclIpAceSourcePortBitmask;
WriteMethod     write_aclIpAceDestPortOp;
WriteMethod     write_aclIpAceMinDestPort;
WriteMethod     write_aclIpAceMaxDestPort;
WriteMethod     write_aclIpAceDestPortBitmask;
WriteMethod     write_aclIpAceControlCode;
WriteMethod     write_aclIpAceControlCodeBitmask;
WriteMethod     write_aclIpAceStatus;

/*aclMacAceTable*/
void            init_aclMacAceTable(void);
FindVarMethod   var_aclMacAceTable;
WriteMethod     write_aclMacAcePktformat;
WriteMethod     write_aclMacAceSourceMacAddr;
WriteMethod     write_aclMacAceSourceMacAddrBitmask;
WriteMethod     write_aclMacAceDestMacAddr;
WriteMethod     write_aclMacAceDestMacAddrBitmask;
WriteMethod     write_aclMacAceVidOp;
WriteMethod     write_aclMacAceMinVid;
WriteMethod     write_aclMacAceVidBitmask;
WriteMethod     write_aclMacAceMaxVid;
WriteMethod     write_aclMacAceEtherTypeOp;
WriteMethod     write_aclMacAceEtherTypeBitmask;
WriteMethod     write_aclMacAceMinEtherType;
WriteMethod     write_aclMacAceMaxEtherType;
WriteMethod     write_aclMacAceStatus;

void            init_aclTable(void);
FindVarMethod   var_aclTable;
WriteMethod     write_aclName;
WriteMethod     write_aclType;
WriteMethod     write_aclStatus;

/* aclAclGroupTable*/
void            init_aclAclGroupTable(void);
FindVarMethod   var_aclAclGroupTable;
WriteMethod     write_aclAclGroupIngressIpAcl;
WriteMethod     write_aclAclGroupEgressIpAcl;
WriteMethod     write_aclAclGroupIngressMacAcl;
WriteMethod     write_aclAclGroupEgressMacAcl;

/* aclIngressMacMaskTable*/
void            init_aclIngressMacMaskTable(void);
FindVarMethod   var_aclIngressMacMaskTable;
WriteMethod     write_aclIngressMacMaskSourceMacAddrBitmask;
WriteMethod     write_aclIngressMacMaskDestMacAddrBitmask;
WriteMethod     write_aclIngressMacMaskVidBitmask;
WriteMethod     write_aclIngressMacMaskEtherTypeBitmask;
WriteMethod     write_aclIngressMacMaskIsEnablePktformat;
WriteMethod     write_aclIngressMacMaskStatus;

/* aclEgressMacMaskTable */
void            init_aclEgressMacMaskTable(void);
FindVarMethod   var_aclEgressMacMaskTable;
WriteMethod     write_aclEgressMacMaskSourceMacAddrBitmask;
WriteMethod     write_aclEgressMacMaskDestMacAddrBitmask;
WriteMethod     write_aclEgressMacMaskVidBitmask;
WriteMethod     write_aclEgressMacMaskEtherTypeBitmask;
WriteMethod     write_aclEgressMacMaskIsEnablePktformat;
WriteMethod     write_aclEgressMacMaskStatus;

/* aclIngressIpMaskTable */
void            init_aclIngressIpMaskTable(void);
FindVarMethod   var_aclIngressIpMaskTable;
WriteMethod     write_aclIngressIpMaskIsEnableTos;
WriteMethod     write_aclIngressIpMaskIsEnableDscp;
WriteMethod     write_aclIngressIpMaskIsEnablePrecedence;
WriteMethod     write_aclIngressIpMaskIsEnableProtocol;
WriteMethod     write_aclIngressIpMaskSourceIpAddrBitmask;
WriteMethod     write_aclIngressIpMaskDestIpAddrBitmask;
WriteMethod     write_aclIngressIpMaskSourcePortBitmask;
WriteMethod     write_aclIngressIpMaskDestPortBitmask;
WriteMethod     write_aclIngressIpMaskControlCodeBitmask;
WriteMethod     write_aclIngressIpMaskStatus;

/* aclEgressIpMaskTable */
void            init_aclEgressIpMaskTable(void);
FindVarMethod   var_aclEgressIpMaskTable;
WriteMethod     write_aclEgressIpMaskIsEnableTos;
WriteMethod     write_aclEgressIpMaskIsEnableDscp;
WriteMethod     write_aclEgressIpMaskIsEnablePrecedence;
WriteMethod     write_aclEgressIpMaskIsEnableProtocol;
WriteMethod     write_aclEgressIpMaskSourceIpAddrBitmask;
WriteMethod     write_aclEgressIpMaskDestIpAddrBitmask;
WriteMethod     write_aclEgressIpMaskSourcePortBitmask;
WriteMethod     write_aclEgressIpMaskDestPortBitmask;
WriteMethod     write_aclEgressIpMaskControlCodeBitmask;
WriteMethod     write_aclEgressIpMaskStatus;

#if (SYS_CPNT_ACL_IPV6 == TRUE)
void            init_aclIpv6AceTable(void);
FindVarMethod   var_aclIpv6AceTable;
WriteMethod     write_aclIpv6AceType;
WriteMethod     write_aclIpv6AceAccess;
WriteMethod     write_aclIpv6AceSourceIpAddr;
WriteMethod     write_aclIpv6AceSourceIpAddrPrefixLen;
WriteMethod     write_aclIpv6AceDestIpAddr;
WriteMethod     write_aclIpv6AceDestIpAddrPrefixLen;

#if (SYS_CPNT_ACL_IPV6_NEXT_HEADER == TRUE)
WriteMethod     write_aclIpv6AceNextHeader;
#endif

WriteMethod     write_aclIpv6AceDscp;

#if (SYS_CPNT_ACL_IPV6_FLOW_LABEL == TRUE)
WriteMethod     write_aclIpv6AceFlowLabel;
#endif

WriteMethod     write_aclIpv6AceStatus;
#endif

/********************************************
 **************aclIpAceTable*****************
 ********************************************
 */
#define ACLIPACENAME                    1
#define ACLIPACEINDEX                   2
#define ACLIPACEPRECEDENCE              3
#define ACLIPACEACTION                  4
#define ACLIPACESOURCEIPADDR            5
#define ACLIPACESOURCEIPADDRBITMASK     6
#define ACLIPACEDESTIPADDR              7
#define ACLIPACEDESTIPADDRBITMASK       8
#define ACLIPACEPROTOCOL                9
#define ACLIPACEPREC                    10
#define ACLIPACETOS                     11
#define ACLIPACEDSCP                    12
#define ACLIPACESOURCEPORTOP            13
#define ACLIPACEMINSOURCEPORT           14
#define ACLIPACEMAXSOURCEPORT           15
#define ACLIPACESOURCEPORTBITMASK       16
#define ACLIPACEDESTPORTOP              17
#define ACLIPACEMINDESTPORT             18
#define ACLIPACEMAXDESTPORT             19
#define ACLIPACEDESTPORTBITMASK         20
#define ACLIPACECONTROLCODE             21
#define ACLIPACECONTROLCODEBITMASK      22
#define ACLIPACESTATUS                  23

/********************************************
 **************aclMacAceTable****************
 ********************************************
 */
#define ACLMACACENAME                   1
#define ACLMACACEINDEX                  2
#define ACLMACACEPRECEDENCE             3
#define ACLMACACEACTION                 4
#define ACLMACACEPKTFORMAT              5
#define ACLMACACESOURCEMACADDR          6
#define ACLMACACESOURCEMACADDRBITMASK   7
#define ACLMACACEDESTMACADDR            8
#define ACLMACACEDESTMACADDRBITMASK     9
#define ACLMACACEVIDOP                  10
#define ACLMACACEMINVID                 11
#define ACLMACACEVIDBITMASK             12
#define ACLMACACEMAXVID                 13
#define ACLMACACEETHERTYPEOP            14
#define ACLMACACEETHERTYPEBITMASK       15
#define ACLMACACEMINETHERTYPE           16
#define ACLMACACEMAXETHERTYPE           17
#define ACLMACACESTATUS                 18

 /********************************************
 ************AclTable****************
 ********************************************
 */
#define ACLINDEX  1
#define ACLNAME   2
#define ACLTYPE    3
#define ACLACEINDEXLIST   4
#define ACLSTATUS 5

/********************************************
 **************aclAclGroupTable**************
 ********************************************
 */
#define ACLACLGROUPIFINDEX              1
#define ACLACLGROUPINGRESSIPACL         2
#define ACLACLGROUPEGRESSIPACL          3
#define ACLACLGROUPINGRESSMACACL        4
#define ACLACLGROUPEGRESSMACACL         5

/********************************************
 *************aclIngressIpMaskTable**********
 ********************************************
 */
#define ACLINGRESSIPMASKINDEX                   1
#define ACLINGRESSIPMASKPRECEDENCE              2
#define ACLINGRESSIPMASKISENABLETOS             3
#define ACLINGRESSIPMASKISENABLEDSCP            4
#define ACLINGRESSIPMASKISENABLEPRECEDENCE      5
#define ACLINGRESSIPMASKISENABLEPROTOCOL        6
#define ACLINGRESSIPMASKSOURCEIPADDRBITMASK     7
#define ACLINGRESSIPMASKDESTIPADDRBITMASK       8
#define ACLINGRESSIPMASKSOURCEPORTBITMASK       9
#define ACLINGRESSIPMASKDESTPORTBITMASK         10
#define ACLINGRESSIPMASKCONTROLCODEBITMASK      11
#define ACLINGRESSIPMASKSTATUS                  12

/********************************************
 ************aclEgressIpMaskTable************
 ********************************************
 */
#define ACLEGRESSIPMASKINDEX                    1
#define ACLEGRESSIPMASKPRECEDENCE               2
#define ACLEGRESSIPMASKISENABLETOS              3
#define ACLEGRESSIPMASKISENABLEDSCP             4
#define ACLEGRESSIPMASKISENABLEPRECEDENCE       5
#define ACLEGRESSIPMASKISENABLEPROTOCOL         6
#define ACLEGRESSIPMASKSOURCEIPADDRBITMASK      7
#define ACLEGRESSIPMASKDESTIPADDRBITMASK        8
#define ACLEGRESSIPMASKSOURCEPORTBITMASK        9
#define ACLEGRESSIPMASKDESTPORTBITMASK          10
#define ACLEGRESSIPMASKCONTROLCODEBITMASK       11
#define ACLEGRESSIPMASKSTATUS                   12

/********************************************
 ************aclIngressMacMaskTable**********
 ********************************************
 */
#define ACLINGRESSMACMASKINDEX                  1
#define ACLINGRESSMACMASKPRECEDENCE             2
#define ACLINGRESSMACMASKSOURCEMACADDRBITMASK   3
#define ACLINGRESSMACMASKDESTMACADDRBITMASK     4
#define ACLINGRESSMACMASKVIDBITMASK             5
#define ACLINGRESSMACMASKETHERTYPEBITMASK       6
#define ACLINGRESSMACMASKISENABLEPKTFORMAT      7
#define ACLINGRESSMACMASKSTATUS                 8


/********************************************
 ************aclEgressMacMaskTable***********
 ********************************************
 */
#define ACLEGRESSMACMASKINDEX                   1
#define ACLEGRESSMACMASKPRECEDENCE              2
#define ACLEGRESSMACMASKSOURCEMACADDRBITMASK    3
#define ACLEGRESSMACMASKDESTMACADDRBITMASK      4
#define ACLEGRESSMACMASKVIDBITMASK              5
#define ACLEGRESSMACMASKETHERTYPEBITMASK        6
#define ACLEGRESSMACMASKISENABLEPKTFORMAT       7
#define ACLEGRESSMACMASKSTATUS                  8

 /********************************************
 ************aclIpv6AceTable*************
 ********************************************
 */
#if (SYS_CPNT_ACL_IPV6 == TRUE)
#define ACLIPV6ACEINDEX		                1
#define ACLIPV6ACETYPE                      2
#define ACLIPV6ACEACCESS		            3
#define ACLIPV6ACESOURCEIPADDR		        4
#define ACLIPV6ACESOURCEIPADDRPREFIXLEN		5
#define ACLIPV6ACEDESTIPADDR		        6
#define ACLIPV6ACEDESTIPADDRPREFIXLEN		7

#if (SYS_CPNT_ACL_IPV6_NEXT_HEADER == TRUE)
#define ACLIPV6ACENEXTHEADER		        8
#endif

#define ACLIPV6ACEDSCP		                9

#if (SYS_CPNT_ACL_IPV6_FLOW_LABEL == TRUE)
#define ACLIPV6ACEFLOWLABEL		            10
#endif

#define ACLIPV6ACESTATUS		            11
#endif


