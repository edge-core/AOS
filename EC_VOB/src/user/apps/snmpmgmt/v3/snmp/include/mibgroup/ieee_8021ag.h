/* Module Name: ieee_8021ag.h
 * Purpose: 802.1ag MIB function
 * Notes: 
 * History:                                                               
 *    2007/4/14 - Daniel, Created
 * Copyright(C)      Accton Corporation, 1999, 2007				
 */

#ifndef IEEE_8021AG_H
#define IEEE_8021AG_H

/* function declarations */

/* dot1agCfmStack Group(1) */
void init_dot1agCfmStackTable(void);
FindVarMethod var_dot1agCfmStackTable;


#define DOT1AGCFMSTACKIFINDEX       1   /* not-accessible */
#define DOT1AGCFMSTACKVLANIDORNONE       2  /* not-accessible */
#define DOT1AGCFMSTACKMDLEVEL       3  /* not-accessible */
#define DOT1AGCFMSTACKDIRECTION       4  /* not-accessible */
#define DOT1AGCFMSTACKMDINDEX       5
#define DOT1AGCFMSTACKMAINDEX       6
#define DOT1AGCFMSTACKMEPID       7
#define DOT1AGCFMSTACKMACADDRESS       8


/* dot1agCfmStack Group(4) */
void init_dot1agCfmConfigErrorListTable(void);
FindVarMethod var_dot1agCfmConfigErrorListTable;

#define DOT1AGCFMCONFIGERRORLISTVID       1 /* not-accessible */
#define DOT1AGCFMCONFIGERRORLISTIFINDEX       2 /* not-accessible */
#define DOT1AGCFMCONFIGERRORLISTERRORTYPE       3


/* dot1agCfmMd Group */
void init_dot1agCfmMd(void);
void init_dot1agCfmMdTable(void);
Netsnmp_Node_Handler get_dot1agCfmMdTableNextIndex;
FindVarMethod var_dot1agCfmMdTable;
WriteMethod write_dot1agCfmMdFormat;
WriteMethod write_dot1agCfmMdName;
WriteMethod write_dot1agCfmMdMdLevel;
WriteMethod write_dot1agCfmMdMhfCreation;
WriteMethod write_dot1agCfmMdMhfIdPermission;
WriteMethod write_dot1agCfmMdStatus;

#define DOT1AGCFM_MD_INDEX 1  /* not-accessible */
#define DOT1AGCFM_MD_FORMAT 2
#define DOT1AGCFM_MD_NAME 3
#define DOT1AGCFM_MD_MDLEVEL 4
#define DOT1AGCFM_MD_MHF_CREATION 5
#define DOT1AGCFM_MD_MHFID_PERMISSION 6
#define DOT1AGCFM_MD_MATABLE_NEXTINDEX 7
#define DOT1AGCFM_MD_ROWSTATUS 8


/* dot1agCfmMa Group */
void init_dot1agCfmMaTable(void);
FindVarMethod var_dot1agCfmMaTable;
WriteMethod write_dot1agCfmMaTable;

#define DOT1AGCFMMAINDEX       1 /* not-accessible */
#define DOT1AGCFMMAPRIMARYVLANID       2
#define DOT1AGCFMMAFORMAT       3
#define DOT1AGCFMMANAME       4
#define DOT1AGCFMMAMHFCREATION       5
#define DOT1AGCFMMAIDPERMISSION       6
#define DOT1AGCFMMACCMINTERVAL       7
#define DOT1AGCFMMANUMBEROFVIDS       8
#define DOT1AGCFMMAROWSTATUS       9


void init_dot1agCfmMaMepListTable(void);
FindVarMethod var_dot1agCfmMaMepListTable;
WriteMethod write_dot1agCfmMaMepListRowStatus;

#define DOT1AGCFMMAMEPLISTIDENTIFIER       1 /* not-accessible */
#define DOT1AGCFMMAMEPLISTROWSTATUS       2


/* dot1agCfmMep Group */
void init_dot1agCfmMepTable(void);
FindVarMethod var_dot1agCfmMepTable;
WriteMethod write_dot1agCfmMepIfIndex;
WriteMethod write_dot1agCfmMepDirection;
WriteMethod write_dot1agCfmMepPrimaryVid;
WriteMethod write_dot1agCfmMepActive;
WriteMethod write_dot1agCfmMepCciEnabled;
WriteMethod write_dot1agCfmMepCcmLtmPriority;
WriteMethod write_dot1agCfmMepLowPrDef;
WriteMethod write_dot1agCfmMepFngAlarmTime;
WriteMethod write_dot1agCfmMepFngResetTime;
WriteMethod write_dot1agCfmMepTransmitLbmStatus;
WriteMethod write_dot1agCfmMepTransmitLbmDestMacAddress;
WriteMethod write_dot1agCfmMepTransmitLbmDestMepId;
WriteMethod write_dot1agCfmMepTransmitLbmDestIsMepId;
WriteMethod write_dot1agCfmMepTransmitLbmMessages;
WriteMethod write_dot1agCfmMepTransmitLbmDataTlv;
WriteMethod write_dot1agCfmMepTransmitLbmVlanPriority;
WriteMethod write_dot1agCfmMepTransmitLbmVlanDropEnable;
WriteMethod write_dot1agCfmMepTransmitLtmFlags;
WriteMethod write_dot1agCfmMepTransmitLtmTargetMacAddress;
WriteMethod write_dot1agCfmMepTransmitLtmTargetMepId;
WriteMethod write_dot1agCfmMepTransmitLtmTargetIsMepId;
WriteMethod write_dot1agCfmMepTransmitLtmTtl;
WriteMethod write_dot1agCfmMepTransmitLtmEgressIdentifier;
WriteMethod write_dot1agCfmMepRowStatus;
WriteMethod write_dot1agCfmMepTransmitLtmStatus;

#define DOT1AGCFMMEPIDENTIFIER       1 /* not-accessible */
#define DOT1AGCFMMEPIFINDEX       2
#define DOT1AGCFMMEPDIRECTION       3
#define DOT1AGCFMMEPPRIMARYVID       4
#define DOT1AGCFMMEPACTIVE       5
#define DOT1AGCFMMEPFNGSTATE       6
#define DOT1AGCFMMEPCCIENABLED       7
#define DOT1AGCFMMEPCCMLTMPRIORITY       8
#define DOT1AGCFMMEPMACADDRESS       9
#define DOT1AGCFMMEPLOWPRDEF       10
#define DOT1AGCFMMEPFNGALARMTIME       11
#define DOT1AGCFMMEPFNGRESETTIME       12
#define DOT1AGCFMMEPHIGHESTPRDEFECT       13
#define DOT1AGCFMMEPDEFECTS       14
#define DOT1AGCFMMEPERRORCCMLASTFAILURE       15
#define DOT1AGCFMMEPXCONCCMLASTFAILURE       16
#define DOT1AGCFMMEPCCMSEQUENCEERRORS       17
#define DOT1AGCFMMEPCCISENTCCMS       18
#define DOT1AGCFMMEPNEXTLBMTRANSID       19
#define DOT1AGCFMMEPLBRIN       20
#define DOT1AGCFMMEPLBRINOUTOFORDER       21
#define DOT1AGCFMMEPLBRBADMSDU       22
#define DOT1AGCFMMEPLTMNEXTSEQNUMBER       23
#define DOT1AGCFMMEPUNEXPLTRIN       24
#define DOT1AGCFMMEPLBROUT       25
#define DOT1AGCFMMEPTRANSMITLBMSTATUS       26
#define DOT1AGCFMMEPTRANSMITLBMDESTMACADDRESS       27
#define DOT1AGCFMMEPTRANSMITLBMDESTMEPID       28
#define DOT1AGCFMMEPTRANSMITLBMDESTISMEPID       29
#define DOT1AGCFMMEPTRANSMITLBMMESSAGES       30
#define DOT1AGCFMMEPTRANSMITLBMDATATLV       31
#define DOT1AGCFMMEPTRANSMITLBMVLANPRIORITY       32
#define DOT1AGCFMMEPTRANSMITLBMVLANDROPENABLE       33
#define DOT1AGCFMMEPTRANSMITLBMRESULTOK       34
#define DOT1AGCFMMEPTRANSMITLBMSEQNUMBER       35
#define DOT1AGCFMMEPTRANSMITLTMSTATUS       36
#define DOT1AGCFMMEPTRANSMITLTMFLAGS       37
#define DOT1AGCFMMEPTRANSMITLTMTARGETMACADDRESS       38
#define DOT1AGCFMMEPTRANSMITLTMTARGETMEPID       39
#define DOT1AGCFMMEPTRANSMITLTMTARGETISMEPID       40
#define DOT1AGCFMMEPTRANSMITLTMTTL       41
#define DOT1AGCFMMEPTRANSMITLTMRESULT       42
#define DOT1AGCFMMEPTRANSMITLTMSEQNUMBER       43
#define DOT1AGCFMMEPTRANSMITLTMEGRESSIDENTIFIER       44
#define DOT1AGCFMMEPROWSTATUS       45

void init_dot1agCfmLtrTable(void);
FindVarMethod var_dot1agCfmLtrTable;

#define DOT1AGCFMLTRSEQNUMBER       1  /* not-accessible */
#define DOT1AGCFMLTRRECEIVEORDER       2  /* not-accessible */
#define DOT1AGCFMLTRTTL       3
#define DOT1AGCFMLTRFORWARDED       4
#define DOT1AGCFMLTRTERMINALMEP       5
#define DOT1AGCFMLTRLASTEGRESSIDENTIFIER       6
#define DOT1AGCFMLTRNEXTEGRESSIDENTIFIER       7
#define DOT1AGCFMLTRRELAY       8
#define DOT1AGCFMLTRCHASSISIDSUBTYPE       9
#define DOT1AGCFMLTRCHASSISID       10
#define DOT1AGCFMLTRMANADDRESSDOMAIN       11
#define DOT1AGCFMLTRMANADDRESS       12
#define DOT1AGCFMLTRINGRESS       13
#define DOT1AGCFMLTRINGRESSMAC       14
#define DOT1AGCFMLTRINGRESSPORTIDSUBTYPE       15
#define DOT1AGCFMLTRINGRESSPORTID       16
#define DOT1AGCFMLTREGRESS       17
#define DOT1AGCFMLTREGRESSMAC       18
#define DOT1AGCFMLTREGRESSPORTIDSUBTYPE       19
#define DOT1AGCFMLTREGRESSPORTID       20
#define DOT1AGCFMLTRORGANIZATIONSPECIFICTLV       21

void init_dot1agCfmMepDbTable(void);
FindVarMethod var_dot1agCfmMepDbTable;

#define DOT1AGCFMMEPDBRMEPIDENTIFIER       1 /* not-accessible */
#define DOT1AGCFMMEPDBRMEPSTATE       2
#define DOT1AGCFMMEPDBRMEPFAILEDOKTIME       3
#define DOT1AGCFMMEPDBMACADDRESS       4
#define DOT1AGCFMMEPDBRDI       5
#define DOT1AGCFMMEPDBPORTSTATUSTLV       6
#define DOT1AGCFMMEPDBINTERFACESTATUSTLV       7
#define DOT1AGCFMMEPDBCHASSISIDSUBTYPE       8
#define DOT1AGCFMMEPDBCHASSISID       9
#define DOT1AGCFMMEPDBMANADDRESSDOMAIN       10
#define DOT1AGCFMMEPDBMANADDRESS       11

#endif
