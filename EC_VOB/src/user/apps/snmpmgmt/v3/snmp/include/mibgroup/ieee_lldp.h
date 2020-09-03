#ifndef _IEEE_LLDP_H
#define _IEEE_LLDP_H

#if (SYS_CPNT_LLDP == TRUE)
void init_lldpConfiguration(void);
Netsnmp_Node_Handler do_lldpMessageTxInterval;
Netsnmp_Node_Handler do_lldpMessageTxHoldMultiplier;
Netsnmp_Node_Handler do_lldpReinitDelay;
Netsnmp_Node_Handler do_lldpTxDelay;
Netsnmp_Node_Handler do_lldpNotificationInterval;

void init_lldpPortConfigTable(void);
FindVarMethod var_lldpPortConfigTable;
WriteMethod write_lldpPortConfigAdminStatus;
WriteMethod write_lldpPortConfigNotificationEnable;
WriteMethod write_lldpPortConfigTLVsTxEnable;

void init_lldpConfigManAddrTable(void);
FindVarMethod var_lldpConfigManAddrTable;
WriteMethod write_lldpConfigManAddrTable;

void init_lldpStatistics(void);
Netsnmp_Node_Handler get_lldpStatsRemTablesLastChangeTime;
Netsnmp_Node_Handler get_lldpStatsRemTablesInserts;
Netsnmp_Node_Handler get_lldpStatsRemTablesDeletes;
Netsnmp_Node_Handler get_lldpStatsRemTablesDrops;
Netsnmp_Node_Handler get_lldpStatsRemTablesAgeouts;

void init_lldpLocalSystemData(void);
Netsnmp_Node_Handler get_lldpLocChassisIdSubtype;
Netsnmp_Node_Handler get_lldpLocChassisId;
Netsnmp_Node_Handler get_lldpLocSysName;
Netsnmp_Node_Handler get_lldpLocSysDesc;
Netsnmp_Node_Handler get_lldpLocSysCapSupported;
Netsnmp_Node_Handler get_lldpLocSysCapEnabled;

void init_lldpStatsTxPortTable(void);
FindVarMethod var_lldpStatsTxPortTable;

void init_lldpStatsRxPortTable(void);
FindVarMethod var_lldpStatsRxPortTable;

void init_lldpLocPortTable(void);
FindVarMethod var_lldpLocPortTable;

void init_lldpLocManAddrTable(void);
FindVarMethod var_lldpLocManAddrTable;

void init_lldpRemTable(void);
FindVarMethod var_lldpRemTable;

void init_lldpRemManAddrTable(void);
FindVarMethod var_lldpRemManAddrTable;

void init_lldpRemUnknownTLVTable(void);
FindVarMethod var_lldpRemUnknownTLVTable;

void init_lldpRemOrgDefInfoTable(void);
FindVarMethod var_lldpRemOrgDefInfoTable;

#if 0
void init_lldpXdot3PortConfigTable(void);
FindVarMethod var_lldpXdot3PortConfigTable;
WriteMethod write_lldpXdot3PortConfigTable;

void init_lldpXdot3LocPortTable(void);
FindVarMethod var_lldpXdot3LocPortTable;
FindVarMethod var_lldpXdot3LocLinkAggTable;
FindVarMethod var_lldpXdot3LocMaxFrameSizeTable;

void init_lldpXdot3RemDataTable(void);
FindVarMethod var_lldpXdot3RemDataTable;
FindVarMethod var_lldpXdot3RemLinkAggTable;
FindVarMethod var_lldpXdot3RemMaxFrameSizeTable;
#endif
#define LLDPPORTCONFIGPORTNUM       1
#define LLDPPORTCONFIGADMINSTATUS       2
#define LLDPPORTCONFIGNOTIFICATIONENABLE       3
#define LLDPPORTCONFIGTLVSTXENABLE       4

#endif
#endif  /* End of _IEEE_LLDP_H*/

