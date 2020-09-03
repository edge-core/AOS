#ifndef  LEAF_SFLOWV5_H
#define  LEAF_SFLOWV5_H
/* sFlow agent
 */
#define LEAF_sFlowAgentAddress   3
#define MINSIZE_sFlowAgentAddress  0L
#define MAXSIZE_sFlowAgentAddress  255L

#define LEAF_sFlowAgentAddressType   2
#define VAL_sFlowAgentAddressType_unknown 0L
#define VAL_sFlowAgentAddressType_ipv4 1L
#define VAL_sFlowAgentAddressType_ipv6 2L
#define VAL_sFlowAgentAddressType_ipv4z 3L
#define VAL_sFlowAgentAddressType_ipv6z 4L
#define VAL_sFlowAgentAddressType_dns 16L
#define LEAF_sFlowVersion   1
#define MINSIZE_sFlowVersion  0L
#define MAXSIZE_sFlowVersion  255L

/* sFlow Rcvr Table
 */
 #define SFLOWRCVRINDEX       1
#define SFLOWRCVROWNER       2
#define SFLOWRCVRTIMEOUT       3
#define SFLOWRCVRMAXIMUMDATAGRAMSIZE       4
#define SFLOWRCVRADDRESSTYPE       5
#define SFLOWRCVRADDRESS       6
#define SFLOWRCVRPORT       7
#define SFLOWRCVRDATAGRAMVERSION       8

#define LEAF_sFlowRcvrIndex   1
#define MIN_sFlowRcvrIndex  1L
#define MAX_sFlowRcvrIndex  65535L
#define LEAF_sFlowRcvrOwner   2
#define MINSIZE_sFlowRcvrOwner  0L
#define MAXSIZE_sFlowRcvrOwner  127L
#define LEAF_sFlowRcvrTimeout   3
#define LEAF_sFlowRcvrMaximumDatagramSize   4
#define LEAF_sFlowRcvrAddressType   5
#define VAL_sFlowRcvrAddressType_unknown 0L
#define VAL_sFlowRcvrAddressType_ipv4 1L
#define VAL_sFlowRcvrAddressType_ipv6 2L
#define VAL_sFlowRcvrAddressType_ipv4z 3L
#define VAL_sFlowRcvrAddressType_ipv6z 4L
#define VAL_sFlowRcvrAddressType_dns 16L
#define LEAF_sFlowRcvrAddress   6
#define MINSIZE_sFlowRcvrAddress  0L
#define MAXSIZE_sFlowRcvrAddress  255L
#define LEAF_sFlowRcvrPort   7
#define LEAF_sFlowRcvrDatagramVersion   8

/* sFlowFsTable
 */
#define SFLOWFSDATASOURCE       1
#define SFLOWFSINSTANCE       2
#define SFLOWFSRECEIVER       3
#define SFLOWFSPACKETSAMPLINGRATE       4
#define SFLOWFSMAXIMUMHEADERSIZE       5

#define LEAF_sFlowFsDataSource   1
#define MINSIZE_sFlowFsDataSource  0L
#define MAXSIZE_sFlowFsDataSource  32L
#define LEAF_sFlowFsInstance   2
#define MIN_sFlowFsInstance  1L
#define MAX_sFlowFsInstance  65535L
#define LEAF_sFlowFsReceiver   3
#define LEAF_sFlowFsPacketSamplingRate   4
#define LEAF_sFlowFsMaximumHeaderSize   5

/* sFlowCPTable
 */
#define SFLOWCPDATASOURCE       1
#define SFLOWCPINSTANCE       2
#define SFLOWCPRECEIVER       3
#define SFLOWCPINTERVAL       4

#define LEAF_sFlowCpDataSource   1
#define MINSIZE_sFlowCpDataSource  0L
#define MAXSIZE_sFlowCpDataSource  32L
#define LEAF_sFlowCpInstance   2
#define MIN_sFlowCpInstance  1L
#define MAX_sFlowCpInstance  65535L
#define LEAF_sFlowCpReceiver   3
#define LEAF_sFlowCpInterval   4
#endif /* LEAF_SFLOWV5_H */
