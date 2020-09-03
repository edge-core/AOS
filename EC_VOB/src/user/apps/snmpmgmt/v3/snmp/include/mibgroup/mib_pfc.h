#ifndef _MIB_PFC_H
#define _MIB_PFC_H
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "snmp_mgr.h"

#if (SYS_CPNT_PFC == TRUE)
/********************************************
 ****************** pfcMgt ******************
 ********************************************
 */
void init_pfcMgt(void);

/********************************************
 *************** pfcPortTable   *************
 ********************************************
 */
void init_pfcPortTable(void);
FindVarMethod var_pfcPortTable;
WriteMethod write_pfcPortMode;
WriteMethod write_pfcPortPriEnableList;

#define PFCPORTINDEX                    1
#define PFCPORTMODE                     2
#define PFCPORTOPERMODE                 3
#define PFCPORTPRIENABLELIST            4
#define PFCPORTOPERPRIENABLELIST        5


/********************************************
 ************** pfcPortStatsTable ***********
 ********************************************
 */
void init_pfcPortStatsTable(void);
FindVarMethod var_pfcPortStatsTable;
WriteMethod write_pfcPortStatsClearAction;

#define PFCPORTSTATSINDEX               1
#define PFCPORTSTATSSENTPRI0PKTS        2
#define PFCPORTSTATSSENTPRI1PKTS        3
#define PFCPORTSTATSSENTPRI2PKTS        4
#define PFCPORTSTATSSENTPRI3PKTS        5
#define PFCPORTSTATSSENTPRI4PKTS        6
#define PFCPORTSTATSSENTPRI5PKTS        7
#define PFCPORTSTATSSENTPRI6PKTS        8
#define PFCPORTSTATSSENTPRI7PKTS        9
#define PFCPORTSTATSRECVPRI0PKTS       10
#define PFCPORTSTATSRECVPRI1PKTS       11
#define PFCPORTSTATSRECVPRI2PKTS       12
#define PFCPORTSTATSRECVPRI3PKTS       13
#define PFCPORTSTATSRECVPRI4PKTS       14
#define PFCPORTSTATSRECVPRI5PKTS       15
#define PFCPORTSTATSRECVPRI6PKTS       16
#define PFCPORTSTATSRECVPRI7PKTS       17
#define PFCPORTSTATSCLEARACTION        18

#endif /* #if (SYS_CPNT_PFC == TRUE) */

#endif /* #ifndef _MIB_PFC_H */

