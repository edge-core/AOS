/* MODULE NAME:  sysmgmt_type.h
 * PURPOSE:
 *  For common definition for sysmgmt.
 * NOTES:
 *
 * HISTORY
 *    2/17/2006 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2006
 */
#ifndef SYSMGMT_TYPE_H
#define SYSMGMT_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

#define SYSMGMT_TYPE_PASSWORD_THRESHOLD_DISABLED               0
#define SYSMGMT_TYPE_EXEC_TIMEOUT_DISABLED                     0
#define SYSMGMT_TYPE_SILENT_TIME_DISABLED                      0

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    SYSMGMT_TYPE_TRACE_ID_SYS_MGR_GETALLTASKSCPUUSAGE = 0
};

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* End of SYSMGMT_TYPE_H */
