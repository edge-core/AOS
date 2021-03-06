/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.iterate_access.conf,v 1.4 2003/07/01 00:15:11 hardaker Exp $
 */
#ifndef NETSNMPHOSTSTABLE_H
#define NETSNMPHOSTSTABLE_H

/** other required module components */
config_require(examples/netSnmpHostsTable_access)
    config_require(examples/netSnmpHostsTable_checkfns)

    /*
     * function declarations 
     */
     void            init_netSnmpHostsTable(void);
     void            initialize_table_netSnmpHostsTable(void);
     Netsnmp_Node_Handler netSnmpHostsTable_handler;


/*
 * column number definitions for table netSnmpHostsTable 
 */
#include "netSnmpHostsTable_columns.h"

/*
 * enum definions 
 */
#include "netSnmpHostsTable_enums.h"

#endif /** NETSNMPHOSTSTABLE_H */
