
#ifndef LLDPXMED_H
#define LLDPXMED_H

#if (SYS_CPNT_LLDP_MED == TRUE)

void init_lldpXMedConfig(void);
Netsnmp_Node_Handler get_lldpXMedLocDeviceClass;
Netsnmp_Node_Handler do_lldpXMedFastStartRepeatCount;

void init_lldpXMedPortConfigTable(void);
FindVarMethod var_lldpXMedPortConfigTable;
WriteMethod write_lldpXMedPortConfigTLVsTxEnable;
WriteMethod write_lldpXMedPortConfigNotifEnable;

void init_lldpXMedLocMediaPolicyTable(void);
FindVarMethod var_lldpXMedLocMediaPolicyTable;

void init_lldpXMedLocalData(void);
Netsnmp_Node_Handler get_lldpXMedLocHardwareRev;
Netsnmp_Node_Handler get_lldpXMedLocFirmwareRev;
Netsnmp_Node_Handler get_lldpXMedLocSoftwareRev;
Netsnmp_Node_Handler get_lldpXMedLocSerialNum;
Netsnmp_Node_Handler get_lldpXMedLocMfgName;
Netsnmp_Node_Handler get_lldpXMedLocModelName;
Netsnmp_Node_Handler get_lldpXMedLocAssetID;

void init_lldpXMedLocLocationTable(void);
FindVarMethod var_lldpXMedLocLocationTable;
WriteMethod write_lldpXMedLocLocationInfo;

void init_lldpXMedLocXPoEData(void);
Netsnmp_Node_Handler get_lldpXMedLocXPoEDeviceType;
Netsnmp_Node_Handler get_lldpXMedLocXPoEPSEPowerSource;
Netsnmp_Node_Handler get_lldpXMedLocXPoEPDPowerReq;
Netsnmp_Node_Handler get_lldpXMedLocXPoEPDPowerSource;
Netsnmp_Node_Handler get_lldpXMedLocXPoEPDPowerPriority;

void init_lldpXMedLocXPoEPSEPortTable(void);
FindVarMethod var_lldpXMedLocXPoEPSEPortTable;

void init_lldpXMedRemCapabilitiesTable(void);
FindVarMethod var_lldpXMedRemCapabilitiesTable;

void init_lldpXMedRemMediaPolicyTable(void);
FindVarMethod var_lldpXMedRemMediaPolicyTable;

void init_lldpXMedRemInventoryTable(void);
FindVarMethod var_lldpXMedRemInventoryTable;

void init_lldpXMedRemLocationTable(void);
FindVarMethod var_lldpXMedRemLocationTable;

void init_lldpXMedRemXPoETable(void);
FindVarMethod var_lldpXMedRemXPoETable;

void init_lldpXMedRemXPoEPSETable(void);
FindVarMethod var_lldpXMedRemXPoEPSETable;

void init_lldpXMedRemXPoEPDTable(void);
FindVarMethod var_lldpXMedRemXPoEPDTable;


#define LLDPXMEDPORTCAPSUPPORTED       1
#define LLDPXMEDPORTCONFIGTLVSTXENABLE       2
#define LLDPXMEDPORTCONFIGNOTIFENABLE       3
#define LLDPXMEDLOCMEDIAPOLICYAPPTYPE       1
#define LLDPXMEDLOCMEDIAPOLICYVLANID       2
#define LLDPXMEDLOCMEDIAPOLICYPRIORITY       3
#define LLDPXMEDLOCMEDIAPOLICYDSCP       4
#define LLDPXMEDLOCMEDIAPOLICYUNKNOWN       5
#define LLDPXMEDLOCMEDIAPOLICYTAGGED       6
#define LLDPXMEDLOCLOCATIONSUBTYPE       1
#define LLDPXMEDLOCLOCATIONINFO       2
#define LLDPXMEDLOCXPOEPSEPORTPOWERAV       1
#define LLDPXMEDLOCXPOEPSEPORTPDPRIORITY       2
#define LLDPXMEDREMCAPSUPPORTED       1
#define LLDPXMEDREMCAPCURRENT       2
#define LLDPXMEDREMDEVICECLASS       3
#define LLDPXMEDREMMEDIAPOLICYAPPTYPE       1
#define LLDPXMEDREMMEDIAPOLICYVLANID       2
#define LLDPXMEDREMMEDIAPOLICYPRIORITY       3
#define LLDPXMEDREMMEDIAPOLICYDSCP       4
#define LLDPXMEDREMMEDIAPOLICYUNKNOWN       5
#define LLDPXMEDREMMEDIAPOLICYTAGGED       6
#define LLDPXMEDREMHARDWAREREV       1
#define LLDPXMEDREMFIRMWAREREV       2
#define LLDPXMEDREMSOFTWAREREV       3
#define LLDPXMEDREMSERIALNUM       4
#define LLDPXMEDREMMFGNAME       5
#define LLDPXMEDREMMODELNAME       6
#define LLDPXMEDREMASSETID       7
#define LLDPXMEDREMLOCATIONSUBTYPE       1
#define LLDPXMEDREMLOCATIONINFO       2
#define LLDPXMEDREMXPOEDEVICETYPE       1
#define LLDPXMEDREMXPOEPSEPOWERAV       1
#define LLDPXMEDREMXPOEPSEPOWERSOURCE       2
#define LLDPXMEDREMXPOEPSEPOWERPRIORITY       3
#define LLDPXMEDREMXPOEPDPOWERREQ       1
#define LLDPXMEDREMXPOEPDPOWERSOURCE       2
#define LLDPXMEDREMXPOEPDPOWERPRIORITY       3

#endif  /* #if (SYS_CPNT_LLDP_MED == TRUE) */

#endif /* LLDPXMED_H */
