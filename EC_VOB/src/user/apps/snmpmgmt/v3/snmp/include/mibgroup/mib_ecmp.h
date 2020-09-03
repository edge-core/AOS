#ifndef _MIB_ECMP_H
#define _MIB_ECMP_H
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "snmp_mgr.h"

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/********************************************
 ***************** ecmpMgt ******************
 ********************************************
 */
void init_ecmpMgt(void);

Netsnmp_Node_Handler do_ecmpBalanceMode;

#define LEAF_ecmpBalanceMode                    1
#define VAL_ecmpBalanceMode_dstIpL4Port         1L
#define VAL_ecmpBalanceMode_hashSelectionList   2L

Netsnmp_Node_Handler do_ecmpHashSelectionListIndex;

#define LEAF_ecmpHashSelectionListIndex         2
#define MIN_ecmpHashSelectionListIndex          0L
#define MAX_ecmpHashSelectionListIndex          4L

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

#endif /* #ifndef _MIB_ECMP_H */

