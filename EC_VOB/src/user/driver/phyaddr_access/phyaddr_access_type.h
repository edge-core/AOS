/* MODULE NAME:  phyaddr_access_type.h
 * PURPOSE:
 *      Define definitions which are private to PHYADDR_ACCESS.
 * NOTES:
 *      None.
 * HISTORY
 *    2007/07/30 - Echo Chen, Created
 *    2008/02/19 - Anzhen Zheng, Modified
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef PHYADDR_ACCESS_TYPE_H
#define PHYADDR_ACCESS_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
enum
{
    PHYADDR_ACCESS_CMD_GET_VIRTUAL_ADDR = 0,
    PHYADDR_ACCESS_CMD_READ,
    PHYADDR_ACCESS_CMD_WRITE
};

/* anzhen.zheng, 2/19/2008 */
enum
{
    PHYSICAL_ADDR_ACCESS_CMD_READ = 0,
    PHYSICAL_ADDR_ACCESS_CMD_WRITE
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* PHYADDR_ACCESS_TYPE_H */

