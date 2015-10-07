#include "mawiextractornode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include <QDebug>


//------------------------------------------------------------------------------
// Constructor and Destructor

CMawiExtractorNode::CMawiExtractorNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CMawiExtractorNode::configure(CNodeConfig &config)
{
    // Set a Description of this node.
    config.setDescription("Remove traffic labeled as an anomaly or suspicious "
                          "from TCP dumps.");

    //Set the category
    config.setCategory("Extractor");

    // Add parameters
    config.addBool("anomalous", "Extract Anomalous Flows", "Remove flows labeled as "
                   "anomalous.", true);
    config.addBool("suspicious", "Extract Suspicious Flows", "Remove flows labeled "
                   "as suspicious", false);

    // Add the gates.
    config.addInput("in-labels", "table");
    config.addInput("in-flows", "table");
    config.addOutput("out", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CMawiExtractorNode::start()
{
    return true;
}

bool CMawiExtractorNode::data(QString gate_name, const CConstDataPointer &data)
{
    bool processed = false;

    if(gate_name == "in-labels" && data->getType() == "table") {
        m_labels_table = data.staticCast<const CTableData>();
        processed = true;
    }
    else if(gate_name == "in-flows" && data->getType() == "table") {
        m_flows_table = data.staticCast<const CTableData>();
        processed = true;
    }

    // If we have received both tables, comence extraction; otherwise, wait.
    if(!m_labels_table.isNull() && !m_flows_table.isNull()) {
        QSharedPointer<CTableData> new_flows = createTable(m_flows_table);
        if(extractFlows(m_flows_table, m_labels_table, new_flows)) {
            // Flows extracted correctly, send them forward.
            // Send the extracted flows.
            commit("out", new_flows);
        }
        else {
            // Problems found extracting flows.
            commitError("out", "Could not extract flows.");
        }
        // After extracting the flows, do not hold to the original flows
        // ... to release memory where appropriate.
        m_flows_table.clear();
    }

    return processed;
}

QSharedPointer<CTableData> CMawiExtractorNode::createTable(
        const QSharedPointer<const CTableData> &old_table)
{
    QSharedPointer<CTableData> new_table = QSharedPointer<CTableData>(
                static_cast<CTableData*>(createData("table")));

    new_table->addHeader(old_table->header());
    new_table->reserveRows(old_table->rowCount());

    return new_table;
}

bool CMawiExtractorNode::extractFlows(
        const QSharedPointer<const CTableData> &old_flows,
        const QSharedPointer<const CTableData> &labels,
        const QSharedPointer<CTableData> &new_flows)
{
    // Keep track of progress
    quint32 progress_total = old_flows->rowCount();
    quint32 progress_step = progress_total / 100;

    // Get the user defined parameters.
    QVariant v_anoamalous = getConfig().getParameter("anomalous")->value;
    QVariant v_suspicious = getConfig().getParameter("suspicious")->value;

    // Get the location of the filed of interest in the table.
    // Time related fields
    qint32 i_timestamp = old_flows->findHeader("Timestamp");
    qint32 i_duration = old_flows->findHeader("DUR");
    // IP & port related fields
    qint32 i_dp = old_flows->findHeader("DP");
    qint32 i_da = old_flows->findHeader("DA");
    qint32 i_da3 = old_flows->findHeader("DA3");
    qint32 i_da2 = old_flows->findHeader("DA2");
    qint32 i_da1 = old_flows->findHeader("DA1");
    qint32 i_da0 = old_flows->findHeader("DA0");
    qint32 i_sp = old_flows->findHeader("SP");
    qint32 i_sa = old_flows->findHeader("SA");
    qint32 i_sa3 = old_flows->findHeader("SA3");
    qint32 i_sa2 = old_flows->findHeader("SA2");
    qint32 i_sa1 = old_flows->findHeader("SA1");
    qint32 i_sa0 = old_flows->findHeader("SA0");
    // Label related fields
    qint32 i_label_type = labels->findHeader("type");
    qint32 i_label_from = labels->findHeader("from");
    qint32 i_label_to = labels->findHeader("to");
    qint32 i_label_da = labels->findHeader("dst_ip");
    qint32 i_label_dp = labels->findHeader("dst_port");
    qint32 i_label_sa = labels->findHeader("src_ip");
    qint32 i_label_sp = labels->findHeader("src_port");

    // Check if all required fields were found.
    if(i_timestamp == -1 || i_duration == -1) {
        return false;
    }
    if(i_da == -1 && (i_da3 == -1 || i_da2 == -1 || i_da1 == -1 || i_da0 == -1)) {
        return false;
    }
    if(i_sa == -1 && (i_sa3 == -1 || i_sa2 == -1 || i_sa1 == -1 || i_sa0 == -1)) {
        return false;
    }
    if(i_dp == -1 || i_sp == -1) {
        return false;
    }
    if(i_label_type == -1 || i_label_da == -1 || i_label_dp == -1 ||
            i_label_sa == -1 || i_label_sp == -1 || i_label_from == -1 ||
            i_label_to == -1) {
        return false;
    }

    // Extract  'anomalous' or 'suspicious' labels as required to speed things when
    // ... we search.
    QList<QList<QVariant>> effective_labels;
    if(v_anoamalous.toBool() && v_suspicious.toBool()) {
        effective_labels = labels->table();
    }
    else {
        for(const QList<QVariant> &row : labels->table()) {
            if(v_suspicious.toBool() && row[i_label_type] == "suspicious") {
                effective_labels.append(row);
            }
            else if(v_anoamalous.toBool() && row[i_label_type] == "anomalous") {
                effective_labels.append(row);
            }
        }
    }

    // Iterate the 'old_flows' extracting the flows the user does not want.
    bool remove_flow;
    quint32 progress_report = 0;
    for(const QList<QVariant> &row : old_flows->table()) {
        // By default do not remove the current flow.
        remove_flow = false;

        // Get the IP addresses of the flow as a single string format.
        QString da = buildIp(row, i_da, i_da3, i_da2, i_da1, i_da0);
        QString sa = buildIp(row, i_sa, i_sa3, i_sa2, i_sa1, i_sa0);

        // Compare this flow with every single record in the effective labels table.
        for(const QList<QVariant> &label_row : effective_labels) {
            // Compare the IPs taking into account that empty IPs match all cases.
            bool da_ip_match = false;
            bool sa_ip_match = false;
            if(label_row[i_label_da] == "") {
                da_ip_match = true;
            } else {
                da_ip_match = (da == label_row[i_label_da]);
            }
            if(label_row[i_label_sa] == "") {
                sa_ip_match = true;
            }
            else {
                sa_ip_match = (sa == label_row[i_label_sa]);
            }

            if(// Check IP match
               da_ip_match &&
               sa_ip_match &&
               // Check ports
               row[i_dp] == label_row[i_label_dp] &&
               row[i_sp] == label_row[i_label_sp]) {

                // If previous things matched, check timestamp (to save conversions).
                double flow_from = row[i_timestamp].toDouble();
                double flow_to = flow_from + row[i_duration].toDouble();
                double label_from = label_row[i_label_from].toDouble();
                double label_to = label_row[i_label_to].toDouble();
                if((flow_from >= label_from && flow_from < label_to) ||
                   (flow_to > label_to && flow_to <= label_from)) {
                    // This is a match, remove the flow.
                    remove_flow = true;
                    break;
                }
            }
        }

        if(!remove_flow) {
            // Copy the old row to the new table if all tests passed.
            QList<QVariant> &new_row = new_flows->newRow();
            new_row = row;
        }

        // Report progress every so often.
        progress_report++;
        if(progress_report % progress_step == 0) {
            qint64 percentage = static_cast<qint64>(progress_report) * 100 /
                    static_cast<qint64>(progress_total);
            setProgress(static_cast<qint8>(percentage));
        }
    }

    setProgress(100);

    return true;
}

QString CMawiExtractorNode::buildIp(const QList<QVariant> &row, qint32 i_ip,
                                    qint32 i_ip3, qint32 i_ip2, qint32 i_ip1,
                                    qint32 i_ip0)
{
    // Verify first if the IP was already provided in the desired format.
    if(i_ip != -1) {
        // Remove leading zeroes from each octet.
        return row[i_ip].toString().replace(QRegExp("\\.[0]+"), ".");
    }

    // Reconstruct the IP. Remove leading zeroes from each octet.
    QStringList ip_list;
    QRegExp regexp("^[0]*");
    ip_list << row[i_ip3].toString().remove(regexp)
            << row[i_ip2].toString().remove(regexp)
            << row[i_ip1].toString().remove(regexp)
            << row[i_ip0].toString().remove(regexp);
    return ip_list.join(".");
}

