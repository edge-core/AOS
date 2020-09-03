#ifndef SFLOW_H
#define SFLOW_H

#include "sys_cpnt.h"

#if (SYS_CPNT_SFLOW == TRUE)
void init_sFlow(void);
Netsnmp_Node_Handler get_sFlowVersion;
Netsnmp_Node_Handler get_sFlowAgentAddressType;
Netsnmp_Node_Handler get_sFlowAgentAddress;
FindVarMethod var_sFlowTable;
WriteMethod write_sFlowOwner;
WriteMethod write_sFlowTimeout;
WriteMethod write_sFlowPacketSamplingRate;
WriteMethod write_sFlowCounterSamplingInterval;
WriteMethod write_sFlowMaximumHeaderSize;
WriteMethod write_sFlowMaximumDatagramSize;
WriteMethod write_sFlowCollectorAddressType;
WriteMethod write_sFlowCollectorAddress;
WriteMethod write_sFlowCollectorPort;
WriteMethod write_sFlowDatagramVersion;
#endif /* end of #if (SYS_CPNT_SFLOW == TRUE) */

#endif /* end of #ifndef SFLOW_H */
