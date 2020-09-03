#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"

#if (SYS_CPNT_SYNCE == TRUE)
void init_syncEStatus(void);
Netsnmp_Node_Handler do_syncEStatus;
void init_syncESsmStatus(void);
Netsnmp_Node_Handler do_syncESsmStatus;
void init_syncEClockSourcePort(void);
Netsnmp_Node_Handler do_syncEClockSourcePort;
void init_syncEGoodClockSource(void);
Netsnmp_Node_Handler get_syncEGoodClockSource;
void init_syncEClockSourceSelect(void);
Netsnmp_Node_Handler do_syncEClockSourceSelect;
void init_syncEAutoClockSourceRevertive(void);
Netsnmp_Node_Handler do_syncEAutoClockSourceRevertive;
void init_syncEClockSourceLocked(void);
Netsnmp_Node_Handler get_syncEClockSourceLocked;
void init_syncEPortTable(void);
FindVarMethod var_syncEPortTable;
WriteMethod write_syncEPortStatus;
WriteMethod write_syncEPortSSMStatus;
WriteMethod write_syncEPortSSMPriority;
WriteMethod write_syncEPortForceClockSourceSelect;

#endif /*end of #if (SYS_CPNT_SYNCE == TRUE)*/
