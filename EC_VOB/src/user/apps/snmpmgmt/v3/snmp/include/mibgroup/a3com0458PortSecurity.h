#if((SYS_CPNT_A3COM0458_PORT_SECURITY_MIB==TRUE) &&(SYS_CPNT_NETWORK_ACCESS==TRUE))
#ifndef A3COM0458PORTSECURITY_H
#define A3COM0458PORTSECURITY_H

void            init_securePort(void);
Netsnmp_Node_Handler get_secureStop;
Netsnmp_Node_Handler do_secureRadaDefaultSessionTime;
Netsnmp_Node_Handler get_securePortVlanMembershipList;
Netsnmp_Node_Handler do_secureRadaHoldoffTime;
Netsnmp_Node_Handler do_secureRadaAuthPassword;
Netsnmp_Node_Handler do_secureRadaAuthMode;
Netsnmp_Node_Handler do_secureRadaReauthenticate;
Netsnmp_Node_Handler do_secureRadaAuthUsername;
Netsnmp_Node_Handler do_securePortSecurityControl;

void            init_securePortTable(void);
FindVarMethod   var_securePortTable;
WriteMethod     write_securePortMode;
WriteMethod     write_secureNeedToKnowMode;
WriteMethod     write_secureIntrusionAction;
WriteMethod     write_secureNumberAddresses;

void            init_secureAddressTable(void);
FindVarMethod   var_secureAddressTable;
WriteMethod     write_secureAddrRowStatus;

void            init_secureOUITable(void);
FindVarMethod   var_secureOUITable;
WriteMethod     write_secureOUIRowStatus;

#endif 
#endif 