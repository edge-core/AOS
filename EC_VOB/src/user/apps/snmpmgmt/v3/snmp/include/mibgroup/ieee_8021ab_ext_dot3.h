#ifndef _IEEE_LLDPEXT3_H
#define _IEEE_LLDPEXT3_H

#if (SYS_CPNT_LLDP_EXT == TRUE)
/* 802.3 extensions */
void init_lldpXdot3PortConfigTable(void);
FindVarMethod var_lldpXdot3PortConfigTable;
WriteMethod write_lldpXdot3PortConfigTLVsTxEnable;

void init_lldpXdot3LocPortTable(void);
FindVarMethod var_lldpXdot3LocPortTable;

void init_lldpXdot3LocPowerTable(void);
FindVarMethod var_lldpXdot3LocPowerTable;

void init_lldpXdot3LocLinkAggTable(void);
FindVarMethod var_lldpXdot3LocLinkAggTable;

void init_lldpXdot3LocMaxFrameSizeTable(void);
FindVarMethod var_lldpXdot3LocMaxFrameSizeTable;

void init_lldpXdot3RemPortTable(void);
FindVarMethod var_lldpXdot3RemPortTable;

void init_lldpXdot3RemPowerTable(void);
FindVarMethod var_lldpXdot3RemPowerTable;

void init_lldpXdot3RemLinkAggTable(void);
FindVarMethod var_lldpXdot3RemLinkAggTable;

void init_lldpXdot3RemMaxFrameSizeTable(void);
FindVarMethod var_lldpXdot3RemMaxFrameSizeTable;

/* 802.3 extensions */
#define LLDPXDOT3PORTCONFIGTLVSTXENABLE       1
#define LLDPXDOT3LOCPORTAUTONEGSUPPORTED       1
#define LLDPXDOT3LOCPORTAUTONEGENABLED       2
#define LLDPXDOT3LOCPORTAUTONEGADVERTISEDCAP       3
#define LLDPXDOT3LOCPORTOPERMAUTYPE       4
#define LLDPXDOT3LOCPOWERPORTCLASS       1
#define LLDPXDOT3LOCPOWERMDISUPPORTED       2
#define LLDPXDOT3LOCPOWERMDIENABLED       3
#define LLDPXDOT3LOCPOWERPAIRCONTROLABLE       4
#define LLDPXDOT3LOCPOWERPAIRS       5
#define LLDPXDOT3LOCPOWERCLASS       6
#define LLDPXDOT3LOCLINKAGGSTATUS       1
#define LLDPXDOT3LOCLINKAGGPORTID       2
#define LLDPXDOT3LOCMAXFRAMESIZE       1
#define LLDPXDOT3REMPORTAUTONEGSUPPORTED       1
#define LLDPXDOT3REMPORTAUTONEGENABLED       2
#define LLDPXDOT3REMPORTAUTONEGADVERTISEDCAP       3
#define LLDPXDOT3REMPORTOPERMAUTYPE       4
#define LLDPXDOT3REMPOWERPORTCLASS       1
#define LLDPXDOT3REMPOWERMDISUPPORTED       2
#define LLDPXDOT3REMPOWERMDIENABLED       3
#define LLDPXDOT3REMPOWERPAIRCONTROLABLE       4
#define LLDPXDOT3REMPOWERPAIRS       5
#define LLDPXDOT3REMPOWERCLASS       6
#define LLDPXDOT3REMLINKAGGSTATUS       1
#define LLDPXDOT3REMLINKAGGPORTID       2
#define LLDPXDOT3REMMAXFRAMESIZE       1

#endif
#endif
