/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.scalar.conf,v 1.5 2002/07/18 14:18:52 dts12 Exp $
 */
#ifndef RADIUSAUTHCLIENT_H
#define RADIUSAUTHCLIENT_H

/* function declarations */
void init_radiusAuthClient(void);
Netsnmp_Node_Handler get_radiusAuthClientIdentifier;
Netsnmp_Node_Handler get_radiusAuthClientInvalidServerAddresses;


/* function declarations */
void init_radiusAuthServerTable(void);
FindVarMethod var_radiusAuthServerTable;
FindVarMethod var_radiusAuthServerTable;

#endif /* RADIUSAUTHCLIENT_H */