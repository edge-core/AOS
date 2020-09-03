#ifndef SFLOWV5_H
#define SFLOWV5_H
void init_sFlowAgent(void);
Netsnmp_Node_Handler get_sFlow5AgentAddress;
Netsnmp_Node_Handler get_sFlow5AgentAddressType;
Netsnmp_Node_Handler get_sFlow5Version;

/* sFlow Rcv Table
 */
void init_sFlowTable(void);
FindVarMethod var_sFlowRcvrTable;
WriteMethod write_sFlowRcvrOwner;
WriteMethod write_sFlowRcvrTimeout;
WriteMethod write_sFlowRcvrMaximumDatagramSize;
WriteMethod write_sFlowRcvrAddressType;
WriteMethod write_sFlowRcvrAddress;
WriteMethod write_sFlowRcvrPort;
WriteMethod write_sFlowRcvrDatagramVersion;

/* sFlowFsTable
 */
FindVarMethod var_sFlowFsTable;
WriteMethod write_sFlowFsReceiver;
WriteMethod write_sFlowFsPacketSamplingRate;
WriteMethod write_sFlowFsMaximumHeaderSize;

/* sFlowCPTable
 */
FindVarMethod var_sFlowCpTable;
WriteMethod write_sFlowCpReceiver;
WriteMethod write_sFlowCpInterval;

#endif /* SFLOWV5_H */
