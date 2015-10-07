#ifndef FLOWTEMPLATE_H
#define FLOWTEMPLATE_H

#include <fixbuf/public.h>

fbInfoElementSpec_t yaf_flow_spec[] = {
    /* Millisecond start and end (epoch) (native time) */
    { (char *)"flowStartMilliseconds",              0, 0 },
    { (char *)"flowEndMilliseconds",                0, 0 },
    /* Counters */
    { (char *)"octetTotalCount",                    0, 0 },
    { (char *)"reverseOctetTotalCount",             0, 0 },
    { (char *)"packetTotalCount",                   0, 0 },
    { (char *)"reversePacketTotalCount",            0, 0 },
    /* 5-tuple and flow status */
    { (char *)"sourceIPv6Address",                  0, 0 },
    { (char *)"destinationIPv6Address",             0, 0 },
    { (char *)"sourceIPv4Address",                  0, 0 },
    { (char *)"destinationIPv4Address",             0, 0 },
    { (char *)"sourceTransportPort",                0, 0 },
    { (char *)"destinationTransportPort",           0, 0 },
    { (char *)"flowAttributes",                     0, 0 },
    { (char *)"reverseFlowAttributes",              0, 0 },
    { (char *)"protocolIdentifier",                 0, 0 },
    { (char *)"flowEndReason",                      0, 0 },
    { (char *)"silkAppLabel",                       0, 0 },
    /* Round-trip time */
    { (char *)"reverseFlowDeltaMilliseconds",       0, 0 },
    { (char *)"tcpSequenceNumber",                  0, 0 },
    { (char *)"reverseTcpSequenceNumber",           0, 0 },
    { (char *)"initialTCPFlags",                    0, 0 },
    { (char *)"unionTCPFlags",                      0, 0 },
    { (char *)"reverseInitialTCPFlags",             0, 0 },
    { (char *)"reverseUnionTCPFlags",               0, 0 },
    { (char *)"vlanId",                             0, 0 },
    { (char *)"reverseVlanId",                      0, 0 },
    { (char *)"ingressInterface",                   0, 0 },
    { (char *)"egressInterface",                    0, 0 },
    { (char *)"subTemplateMultiList",               0, 0 },
    FB_IESPEC_NULL
};

typedef struct yaf_flow_st {
    uint64_t    flowStartMilliseconds;
    uint64_t    flowEndMilliseconds;
    uint64_t    octetTotalCount;
    uint64_t    reverseOctetTotalCount;
    uint64_t    packetTotalCount;
    uint64_t    reversePacketTotalCount;
    uint8_t     sourceIPv6Address[16];
    uint8_t     destinationIPv6Address[16];
    uint32_t    sourceIPv4Address;
    uint32_t    destinationIPv4Address;
    uint16_t    sourceTransportPort;
    uint16_t    destinationTransportPort;
    uint16_t    flowAttributes;
    uint16_t    reverseFlowAttributes;
    uint8_t     protocolIdentifier;
    uint8_t     flowEndReason;
    uint16_t    silkAppLabel;
    int32_t     reverseFlowDeltaMilliseconds;
    uint32_t    tcpSequenceNumber;
    uint32_t    reverseTcpSequenceNumber;
    uint8_t     initialTCPFlags;
    uint8_t     unionTCPFlags;
    uint8_t     reverseInitialTCPFlags;
    uint8_t     reverseUnionTCPFlags;
    uint16_t    vlanId;
    uint16_t    reverseVlanId;
    uint32_t    ingress;
    uint32_t    egress;
    fbSubTemplateMultiList_t subTemplateMultiList;
} yaf_flow_t;

static fbInfoElementSpec_t yaf_stats_spec[] = {
    { (char *)"systemInitTimeMilliseconds",         0, 0 },
    { (char *)"exportedFlowRecordTotalCount",       0, 0 },
    { (char *)"packetTotalCount",                   0, 0 },
    { (char *)"droppedPacketTotalCount",            0, 0 },
    { (char *)"ignoredPacketTotalCount",            0, 0 },
    { (char *)"notSentPacketTotalCount",            0, 0 },
    { (char *)"expiredFragmentCount",               0, 0 },
    { (char *)"assembledFragmentCount",             0, 0 },
    { (char *)"flowTableFlushEventCount",           0, 0 },
    { (char *)"flowTablePeakCount",                 0, 0 },
    { (char *)"exporterIPv4Address",                0, 0 },
    { (char *)"exportingProcessId",                 0, 0 },
    { (char *)"meanFlowRate",                       0, 0 },
    { (char *)"meanPacketRate",                     0, 0 },
    FB_IESPEC_NULL
};

typedef struct yaf_stats_st {
    uint64_t    sysuptime;
    uint64_t    exportedFlowTotalCount;
    uint64_t    packetTotalCount;
    uint64_t    droppedPacketTotalCount;
    uint64_t    ignoredPacketTotalCount;
    uint64_t    notSentPacketTotalCount;
    uint32_t    expiredFragmentCount;
    uint32_t    assembledFragmentCount;
    uint32_t    flowTableFlushEvents;
    uint32_t    flowTablePeakCount;
    uint32_t    exporterIPv4Address;
    uint32_t    exportingProcessId;
    uint32_t    meanFlowRate;
    uint32_t    meanPacketRate;
} yaf_stats_t;


#endif // FLOWTEMPLATE_H
