#ifndef _MIB_ETS_H
#define _MIB_ETS_H
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "snmp_mgr.h"

#if (SYS_CPNT_ETS == TRUE)
/********************************************
 ****************** etsMgt ******************
 ********************************************
 */
void init_etsMgt(void);

/********************************************
 *************** etsPortTable   *************
 ********************************************
 */
void init_etsPortTable(void);
FindVarMethod var_etsPortTable;
WriteMethod write_etsPortMode;

#define ETSPORTINDEX            1
#define ETSPORTMODE             2
#define ETSPORTOPERMODE         3

/********************************************
 ****** etsPortPriorityAssignmentTable ******
 ********************************************
 */
void init_etsPortPriorityAssignmentTable(void);
FindVarMethod var_etsPortPriorityAssignmentTable;
WriteMethod write_etsPortPaTrafficClass;

#define ETSPORTPAINDEX                  1
#define ETSPORTPAPRIORITY               2
#define ETSPORTPATRAFFICCLASS           3
#define ETSPORTPAOPERTRAFFICCLASS       4

/********************************************
 ****** etsPortTrafficClassWeightTable ******
 ********************************************
 */
void init_etsPortTrafficClassWeightTable(void);
FindVarMethod var_etsPortTrafficClassWeightTable;
WriteMethod write_etsPortTcwWeightList;

#define ETSPORTTCWINDEX                 1
#define ETSPORTTCWEIGHTLIST             2
#define ETSPORTTCBOPERWEIGHTLIST        3

/********************************************
 ** etsPortTrafficSelectionAlgorithmTable ***
 ********************************************
 */
void init_etsPortTrafficSelectionAlgorithmTable(void);
FindVarMethod var_etsPortTrafficSelectionAlgorithmTable;
WriteMethod write_etsPortTsaTrafficSelectionAlgorithm;

#define ETSPORTTSAINDEX                             1
#define ETSPORTTSATRAFFICCLASS                      2
#define ETSPORTTSATRAFFICSELECTIONALGORITHM         3
#define ETSPORTTSAOPERTRAFFICSELECTIONALGORITHM     4

#endif /* #if (SYS_CPNT_ETS == TRUE) */

#endif /* #ifndef _MIB_ETS_H */

