#include "tcpstreamextractornode.h"
#include "tcpdumpdata/tcpdumpdata.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include <QDebug>
#include <QHostAddress>


//------------------------------------------------------------------------------
// Constructor and Destructor

CTcpStreamExtractorNode::CTcpStreamExtractorNode(const CNodeConfig &config,
                                                 QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CTcpStreamExtractorNode::configure(CNodeConfig &config)
{
    config.setDescription("Extract streams from TCP packets.");

    // Add parameters
    config.addUInt("payload_size", "TCP Stream Data Size",
                   "The maximum number of bytes that will be stored for "
                   "the contents of a TCP stream.",
                   102400);
    config.addBool("dest_filter", "Should destination IPs be filtered?",
                   "Filter specifiying if IP addresses are filtered "
                   "for analysis.", false);
    config.addString("dest_ip_filter_from", "Starting Valid Destination IP Address",
                     "The first IP value which will be considered "
                     "for the analysis.");
    config.addString("dest_ip_filter_to", "End Valid Destination IP Address",
                     "The last IP value which will be taken into account.");
    config.addUInt("dest_port_filter_from", "Starting Valid Destination Port",
                   "Packages that target a destination port above this "
                   "parameter are accepted.",0);
    config.addUInt("dest_port_filter_to", "Last Valid Destination Port",
                   "Packages targeting a destination port below this "
                   "parameter are accepted", 1024);

    // Add the gates.
    config.addInput("in", "tcpdump");
    config.addOutput("out", "tcpstreams");
}


//------------------------------------------------------------------------------
// Protected Functions


bool CTcpStreamExtractorNode::start()
{
    // Get some user parameters.
    QVariant payload_size = getConfig().getParameter("payload_size")->value;

    // Create a new Data Structure that will hold all data streams.
    m_tcp_streams = QSharedPointer<CTcpStreamsData>(
                static_cast<CTcpStreamsData *>(createData("tcpstreams")));
    m_tcp_streams->setMaxPayloadSize(payload_size.toUInt());

    if(!m_tcp_streams.isNull()) {
        return true;
    }
    else {
        return false;
    }
}

bool CTcpStreamExtractorNode::data(QString gate_name,
                                   const CConstDataPointer &data)
{
    // No need to track gates.
    Q_UNUSED(gate_name);
    QString info;

    if(data->getType() == "tcpdump") {
        auto tcp_dump = data.staticCast<const CTcpDumpData>();

        // User parameters.
        QHostAddress addr_from;
        QHostAddress addr_to;
        quint32 ip_from = 0;
        quint32 ip_to = 0;
        quint16 port_from = 0;
        quint16 port_to = 0;

        bool dest_filter = getConfig().getParameter("dest_filter")->value.toBool();
        if(dest_filter) {
            // Get the destination IP to ranges to filter.
            QString dest_filter_from = getConfig().
                    getParameter("dest_ip_filter_from")->value.toString();
            QString dest_filter_to = getConfig().
                    getParameter("dest_ip_filter_to")->value.toString();
            addr_from = dest_filter_from;
            addr_to = dest_filter_to;
            ip_from = addr_from.toIPv4Address();
            ip_to = addr_to.toIPv4Address();

            // Get the destination port range to filter.
            port_from = getConfig().
                    getParameter("dest_port_filter_from")->value.toUInt();
            port_to = getConfig().
                    getParameter("dest_port_filter_to")->value.toUInt();
        }

        qint32 packet_count = tcp_dump->availablePackets();
        for(qint32 i = 0; i < packet_count; ++i) {
            QSharedPointer<const CTcpDumpPacket> packet = tcp_dump->getPacket(i);
            if(dest_filter) {
                if(packet->dest() < ip_from || packet->dest() >= ip_to ||
                   packet->dest_port() < port_from ||
                   packet->dest_port() > port_to) {
                    // Skip this package as it's outside the filter range.
                    continue;
                }
            }
            m_tcp_streams->addTcpPacket(packet);
        }

        qDebug() << "TCP streams left open:" << m_tcp_streams->openStreamsCount();
        qDebug() << "TCP streams closed:" << m_tcp_streams->closedStreamsCount();
        info="TCP streams left open:" + QVariant(m_tcp_streams->openStreamsCount()).toString();
        setLogInfo(info);
        info="TCP streams closed:" + QVariant(m_tcp_streams->closedStreamsCount()).toString();
        setLogInfo(info);        

        commit("out", m_tcp_streams);
        // Clear the memory used by m_tcp_streams.
        m_tcp_streams.clear();
        return true;
    }

    return false;
}
