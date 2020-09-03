#ifndef A3COM515SNTP_H
#define A3COM515SNTP_H
/*
 * function declarations 
 */
void            init_a3ComSntpGroup(void);
Netsnmp_Node_Handler do_sntpSecondaryServer;
Netsnmp_Node_Handler do_dstStartDate;
Netsnmp_Node_Handler do_timeZoneOffset;
Netsnmp_Node_Handler do_sntpState;
Netsnmp_Node_Handler do_dstState;
Netsnmp_Node_Handler get_sntpActiveServer;
Netsnmp_Node_Handler get_sntpTimeISO;
Netsnmp_Node_Handler do_sntpPollIntervalA3com;
Netsnmp_Node_Handler do_dstEndDate;
Netsnmp_Node_Handler do_sntpPrimaryServer;

#endif
