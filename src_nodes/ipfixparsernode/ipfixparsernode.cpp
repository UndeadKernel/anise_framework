#include "ipfixparsernode.h"
#include "flowtemplate.h"
#include "cert_ie.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "filedata/filedata.h"
#include <QDebug>
#include <glib.h>


//------------------------------------------------------------------------------
// Constructor and Destructor

CIpfixparserNode::CIpfixparserNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CIpfixparserNode::configure(CNodeConfig &config)
{
    // Set a Description of this node.
    config.setDescription("Receive a IPFIX file and parse its contents into a table");

    // Add parameters
    //config.addFilename("file", "Input File", "File to be read from disk.");

    // Add the gates.
    config.addInput("in", "file");
    config.addOutput("flows", "table");
    config.addOutput("stats", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CIpfixparserNode::start()
{
    bool success;
    GError *error = nullptr;

    m_info_model = fbInfoModelAlloc();
    fbInfoModelAddElementArray(m_info_model, yaf_info_elements);

    m_default_template = fbTemplateAlloc(m_info_model);
    success = fbTemplateAppendSpecArray(m_default_template, yaf_flow_spec, 0xffffffff, &error);
    if(!success) {
        logError("Error building the flows template.");
        return false;
    }

    m_stats_template = fbTemplateAlloc(m_info_model);
    success = fbTemplateAppendSpecArray(m_stats_template, yaf_stats_spec, 0xffffffff, &error);
    if(!success) {
        logError("Error building the statistics template.");
        return false;
    }

    m_session = fbSessionAlloc(m_info_model);
    m_flows_template_id = fbSessionAddTemplate(m_session, true, FB_TID_AUTO, m_default_template, &error);
    if(!m_flows_template_id) {
        logError("Could not associate flows template with the session.");
        return false;
    }
    m_stats_template_id = fbSessionAddTemplate(m_session, true, FB_TID_AUTO, m_stats_template, &error);
    if(!m_stats_template_id) {
        logError("Could not associate stats template with the session.");
        return false;
    }

    m_buf = fBufAllocForCollection(m_session, nullptr);
    success = fBufSetInternalTemplate(m_buf, m_flows_template_id, &error);
    if(!success) {
        logError("Could not set internal buffer representation.");
        return false;
    }

    return true;
}

bool CIpfixparserNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);

    QSharedPointer<const CFileData> file;
    bool success = false;
    GError *error = nullptr;
    fbTemplate_t *external_template;
    quint16 external_template_id = 0;
    int num_stats = 0;
    int num_flows = 0;
    std::size_t total_buffer_size;
    std::size_t current_buffer_size;

    if(data->getType() == "file") {

        file = data.staticCast<const CFileData>();
        // Set the reading buffer of the FixBuf library. FixBuf is not const correct, so strip the
        // ... const and reinterpret the pointer to what it expects.x
        char *file_data = const_cast<char *>(file->getBytes().data());
        fBufSetBuffer(m_buf, reinterpret_cast<unsigned char *>(file_data), file->numBytes());
        total_buffer_size = fBufRemaining(m_buf);

        setProgress(0);

        // Set the template to be prepared to receive flow data.
        success = fBufSetInternalTemplate(m_buf, m_flows_template_id, &error);
        if(!success) {
            commitError("out", "Could not set template of IPFIX buffer.");
            g_clear_error(&error);
            return true;
        }

        // Init the table where the flows and the stats will be saved.
        QSharedPointer<CTableData> flows_table = QSharedPointer<CTableData>(
                    static_cast<CTableData *>(createData("table")));
        initSpecsTable(flows_table, yaf_flow_spec);
        QSharedPointer<CTableData> stats_table = QSharedPointer<CTableData>(
                    static_cast<CTableData *>(createData("table")));
        initSpecsTable(stats_table, yaf_stats_spec);

        // Read the flows. Expect both network flows and statistics.
        while(true) {
            // Obtain statistics if there are any.
            external_template = fBufNextCollectionTemplate(m_buf, &external_template_id, &error);
            if(external_template) {
                // There is a template waiting for us.
                // Is the template a statistics template?
                if(fbTemplateGetOptionsScope(external_template) != 0) {
                    // Get ready to parse the statistics with the stats template.
                    if(!fBufSetInternalTemplate(m_buf, m_stats_template_id, &error)) {
                        logError("Could not set the statistics template on buffer.");
                        commitError("out", "Parsing error.");
                        return true;
                    }
                    yaf_stats_t stats;
                    ulong len = sizeof(stats);
                    success = fBufNext(m_buf, (quint8 *)&stats, &len, &error);
                    if(!success) {
                        // Ignore that we couldn't read statistics.
                        logWarning(QString("Could not read flow statistics: %1").arg(error->message));
                        g_clear_error(&error);
                    } else {
                        // Store the statistics in a table
                        QList<QVariant> &row = stats_table->newRow();
                        row.append((quint64)stats.sysuptime);
                        row.append((quint64)stats.exportedFlowTotalCount);
                        row.append((quint64)stats.packetTotalCount);
                        row.append((quint64)stats.droppedPacketTotalCount);
                        row.append((quint64)stats.ignoredPacketTotalCount);
                        row.append((quint64)stats.notSentPacketTotalCount);
                        row.append((quint32)stats.expiredFragmentCount);
                        row.append((quint32)stats.assembledFragmentCount);
                        row.append((quint32)stats.flowTableFlushEvents);
                        row.append((quint32)stats.flowTablePeakCount);
                        row.append((quint32)stats.exporterIPv4Address);
                        row.append((quint32)stats.exportingProcessId);
                        row.append((quint32)stats.meanFlowRate);
                        row.append((quint32)stats.meanPacketRate);

                        ++num_stats;
                    }


                    // Return the template back to the flows.
                    if(!fBufSetInternalTemplate(m_buf, m_flows_template_id, &error)) {
                        logError("Could not return to the flow template.");
                        commitError("out", "Parsing error.");
                        return true;
                    }
                    continue;
                }
            } else {
                // No template and no message.
                if(g_error_matches(error, FB_ERROR_DOMAIN, FB_ERROR_BUFSZ)) {
                    // Finished processing. Commit and quit.
                    fBufFree(m_buf);
                    g_clear_error(&error);
                    break;
                } else {
                    logWarning(error->message);
                    g_clear_error(&error);
                    continue;
                }
            }

            // Obtain network flows.
            yaf_flow_t flows;
            ulong len = sizeof(flows);
            success = fBufNext(m_buf, (quint8 *)&flows, &len, &error);
            if(!success) {
                if(g_error_matches(error, FB_ERROR_DOMAIN, FB_ERROR_EOF)) {
                    // Finished processing flows. Commit and quit.
                    fBufFree(m_buf);
                    g_clear_error(&error);
                    break;
                } else {
                    logWarning(QString("Flows error: %1").arg(error->message));
                    g_clear_error(&error);
                    continue;
                }
            }
            // Store the flow in the table of flows.
            QList<QVariant> &row = flows_table->newRow();
            row.append((quint64)flows.flowStartMilliseconds);
            row.append((quint64)flows.flowEndMilliseconds);
            row.append((quint64)flows.octetTotalCount);
            row.append((quint64)flows.reverseOctetTotalCount);
            row.append((quint64)flows.packetTotalCount);
            row.append((quint64)flows.reversePacketTotalCount);
            row.append(ipv6ToString(flows.sourceIPv6Address));
            row.append(ipv6ToString(flows.destinationIPv6Address));
            row.append((quint32)flows.sourceIPv4Address);
            row.append((quint32)flows.destinationIPv4Address);
            row.append((quint16)flows.sourceTransportPort);
            row.append((quint16)flows.destinationTransportPort);
            row.append((quint16)flows.flowAttributes);
            row.append((quint16)flows.reverseFlowAttributes);
            row.append((quint8)flows.protocolIdentifier);
            row.append((quint8)flows.flowEndReason);
            row.append((quint16)flows.silkAppLabel);
            row.append((qint32)flows.reverseFlowDeltaMilliseconds);
            row.append((quint32)flows.tcpSequenceNumber);
            row.append((quint32)flows.reverseTcpSequenceNumber);
            row.append((quint8)flows.initialTCPFlags);
            row.append((quint8)flows.unionTCPFlags);
            row.append((quint8)flows.reverseInitialTCPFlags);
            row.append((quint8)flows.reverseUnionTCPFlags);
            row.append((quint16)flows.vlanId);
            row.append((quint16)flows.reverseVlanId);
            row.append((quint32)flows.ingress);
            row.append((quint32)flows.egress);

            ++num_flows;

            // Print the progress of the parsing now and then.
            if(num_flows % 1000 == 0) {
                current_buffer_size = fBufRemaining(m_buf);
                setProgress(100 - (100.0f * ((double)current_buffer_size) / ((double)total_buffer_size)));
            }
        }

        logInfo(QString("Num flows: %1").arg(num_flows));
        logInfo(QString("Num stats: %1").arg(num_stats));

        // Sort the flows by start time (field 0).
        setProgress(99);
        flows_table->sort(0);
        setProgress(100);

        commit("flows", flows_table);
        commit("stats", stats_table);

        return true;
    }

    return false;
}

void CIpfixparserNode::initSpecsTable(QSharedPointer<CTableData> flows_table, fbInfoElementSpec_t specs[])
{
    fbInfoElementSpec_t *spec;

    // iterate all the SPECS of the templapte, ignore the last two.
    spec = specs;
    while(spec->name != NULL && strcmp(spec->name, "subTemplateMultiList") != 0) {
        flows_table->addHeader(spec->name);
        spec++;
    }
}


QString CIpfixparserNode::ipv6ToString(quint8 *ip_addr)
{
    char buffer[40];

    char *cp = buffer;
    quint16 *aqp = (quint16 *)ip_addr;
    quint16 aq;
    bool colon_start = FALSE;
    bool colon_end = FALSE;

    for (; (quint8 *)aqp < ip_addr + 16; ++aqp) {
        aq = g_ntohs(*aqp);
        if (aq || colon_end) {
            if ((quint8 *)aqp < ip_addr + 14) {
                snprintf(cp, 6, "%04hx:", aq);
                cp += 5;
            } else {
                snprintf(cp, 5, "%04hx", aq);
                cp += 4;
            }
            if (colon_start) {
                colon_end = TRUE;
            }
        } else if (!colon_start) {
            if ((quint8 *)aqp == ip_addr) {
                snprintf(cp, 3, "::");
                cp += 2;
            } else {
                snprintf(cp, 2, ":");
                cp += 1;
            }
            colon_start = TRUE;
        }
    }

    QString ipv6_string = QString(buffer);
    return ipv6_string;
}
