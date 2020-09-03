#ifndef _IEEE_LLDPEXT_H
#define _IEEE_LLDPEXT_H

#if (SYS_CPNT_LLDP_EXT == TRUE)
/* 802.1 extensions */
void init_lldpXdot1ConfigPortVlanTable(void);
FindVarMethod var_lldpXdot1ConfigPortVlanTable;
WriteMethod write_lldpXdot1ConfigPortVlanTxEnable;

void init_lldpXdot1ConfigVlanNameTable(void);
FindVarMethod var_lldpXdot1ConfigVlanNameTable;
WriteMethod write_lldpXdot1ConfigVlanNameTxEnable;

void init_lldpXdot1ConfigProtoVlanTable(void);
FindVarMethod var_lldpXdot1ConfigProtoVlanTable;
WriteMethod write_lldpXdot1ConfigProtoVlanTxEnable;

void init_lldpXdot1ConfigProtocolTable(void);
FindVarMethod var_lldpXdot1ConfigProtocolTable;
WriteMethod write_lldpXdot1ConfigProtocolTxEnable;

void init_lldpXdot1LocTable(void);
FindVarMethod var_lldpXdot1LocTable;

void init_lldpXdot1LocProtoVlanTable(void);
FindVarMethod var_lldpXdot1LocProtoVlanTable;

void init_lldpXdot1LocVlanNameTable(void);
FindVarMethod var_lldpXdot1LocVlanNameTable;

void init_lldpXdot1LocProtocolTable(void);
FindVarMethod var_lldpXdot1LocProtocolTable;

void init_lldpXdot1RemTable(void);
FindVarMethod var_lldpXdot1RemTable;

void init_lldpXdot1RemProtoVlanTable(void);
FindVarMethod var_lldpXdot1RemProtoVlanTable;

void init_lldpXdot1RemVlanNameTable(void);
FindVarMethod var_lldpXdot1RemVlanNameTable;

void init_lldpXdot1RemProtocolTable(void);
FindVarMethod var_lldpXdot1RemProtocolTable;

/* 802.1 extensions*/
#define LLDPXDOT1CONFIGPORTVLANTXENABLE       1
#define LLDPXDOT1CONFIGVLANNAMETXENABLE       1
#define LLDPXDOT1CONFIGPROTOVLANTXENABLE       1
#define LLDPXDOT1CONFIGPROTOCOLTXENABLE       1
#define LLDPXDOT1LOCPORTVLANID       1
#define LLDPXDOT1LOCPROTOVLANID       1
#define LLDPXDOT1LOCPROTOVLANSUPPORTED       2
#define LLDPXDOT1LOCPROTOVLANENABLED       3
#define LLDPXDOT1LOCVLANID       1
#define LLDPXDOT1LOCVLANNAME       2
#define LLDPXDOT1LOCPROTOCOLINDEX       1
#define LLDPXDOT1LOCPROTOCOLID       2
#define LLDPXDOT1REMPORTVLANID       1
#define LLDPXDOT1REMPROTOVLANID       1
#define LLDPXDOT1REMPROTOVLANSUPPORTED       2
#define LLDPXDOT1REMPROTOVLANENABLED       3
#define LLDPXDOT1REMVLANID       1
#define LLDPXDOT1REMVLANNAME       2
#define LLDPXDOT1REMPROTOCOLINDEX       1
#define LLDPXDOT1REMPROTOCOLID       2

#endif
#endif
