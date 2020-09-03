/* MODULE NAME:  dying_gasp_type.h
 * PURPOSE:
 *     Dying gasp is a feature that will send trap just
 *     before power failure event occurs.
 *
 * NOTES:
 *     None
 *
 * CREATOR:      Charlie Chen
 * HISTORY
 *    7/29/2010 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Corporation, 2010
 */
#ifndef DYING_GASP_TYPE_H
#define DYING_GASP_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
enum {
    DYING_GASP_TYPE_SYSCALL_CMD_WAIT_EVENT=0,
    DYING_GASP_TYPE_SYSCALL_CMD_EMULATE_INTERRUPT,
    DYING_GASP_TYPE_SYSCALL_CMD_SHOW_TIMESTAMP_TO_DEATH,
    DYING_GASP_TYPE_SYSCALL_CMD_ENABLE_INTERRUPT
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* End of DYING_GASP_TYPE_H */
