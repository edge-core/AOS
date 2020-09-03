#ifndef PETHPSEPORTTABLE_H
#define PETHPSEPORTTABLE_H

/*
 * function declarations 
 */
void            init_pethPsePortTable(void);
FindVarMethod   var_pethPsePortTable;
WriteMethod     write_pethPsePortAdminEnable;
WriteMethod     write_pethPsePortPowerPairs;
WriteMethod     write_pethPsePortPowerPriority;
WriteMethod     write_pethPsePortType;

void            init_pethMainPseTable(void);
FindVarMethod   var_pethMainPseTable;
WriteMethod     write_pethMainPseUsageThreshold;

void            init_pethNotificationControlTable(void);
FindVarMethod   var_pethNotificationControlTable;
WriteMethod     write_pethNotificationControlEnable;

#endif                          /* PETHPSEPORTTABLE_H */
