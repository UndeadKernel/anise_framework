#include "mawilabelsnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "filedata/filedata.h"
#include <QDebug>
#include <QXmlStreamReader>


//------------------------------------------------------------------------------
// Constructor and Destructor

CMawiLabelsNode::CMawiLabelsNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CMawiLabelsNode::configure(CNodeConfig &config)
{
    // Set a Description of this node.
    config.setDescription("Parse a file as an XML of labeled anomalies "
                          "from the MAWI dataset.");

    // Add parameters
    //config.addFilename("input_file", "XML File", "XML file containing the anomalies.");

    // Add the gates.
    config.addInput("in", "file");
    config.addOutput("out", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CMawiLabelsNode::start()
{
    return true;
}

bool CMawiLabelsNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);

    // Process input files.
    if(data->getType() == "file") {
        // The file to read.
        auto p_file = data.staticCast<const CFileData>();
        // The table to output the anomalies.
        QSharedPointer<CTableData> table_data = createTable();
        // Place the anomaly labels of the XML file into a table.
        if(parseMawiXml(p_file->getBytes(), table_data)) {
            // Pass the table of anomalies forward.
            commit("out", table_data);
        }
        else {
            commitError("out", "MAWI labels could not be parsed correctly.");
        }

        return true;
    }

    return false;
}

QSharedPointer<CTableData> CMawiLabelsNode::createTable()
{
    QSharedPointer<CTableData> table_data(
                static_cast<CTableData*>(createData("table")));
    // Set the contents of each table we will use.
    table_data->addHeader("number");
    table_data->addHeader("type");
    table_data->addHeader("value");
    table_data->addHeader("from");
    table_data->addHeader("to");
    table_data->addHeader("src_ip");
    table_data->addHeader("src_port");
    table_data->addHeader("dst_ip");
    table_data->addHeader("dst_port");

    return table_data;
}


//------------------------------------------------------------------------------
// Private Functions

bool CMawiLabelsNode::parseMawiXml(const QByteArray &bytes,
                                   QSharedPointer<CTableData> &table_data)
{
    QXmlStreamReader xml(bytes);
    qint32 number = -1;          // The anomaly number (index)
    QString type;                // Either 'anomalous' or 'suspicious'
    QString value;
    double from = -1.0;          // Starting time of the anomaly
    double to = -1.0;            // End time of the anomaly
    QList<QList<QString>> flows; // The flows involved in one anomaly
    QString src_ip;
    QString src_port;
    QString dst_ip;
    QString dst_port;

    while(!xml.atEnd()) {
        xml.readNext();
        if(xml.isStartElement()) {
            if(xml.name() == "anomaly") {
                // Set the initial attributes of the 'anomaly'.
                number++;
                type = xml.attributes().value("type").toString();
                value = xml.attributes().value("value").toString();
                from = -1.0;
                to = -1.0;
                flows.clear();
                // Read all the contents inside the 'anomaly' tag.
                while(!xml.atEnd()) {
                    xml.readNext();
                    // The start of an element.
                    if(xml.isStartElement()) {
                        if(xml.name() == "from") {
                            from = xml.attributes().value("sec").toDouble() +
                                    (xml.attributes().value("usec").toDouble() * 0.000001);
                        }
                        else if(xml.name() == "to") {
                            to = xml.attributes().value("sec").toDouble() +
                                    (xml.attributes().value("usec").toDouble() * 0.000001);
                        }
                        else if(xml.name() == "filter") {
                            // src_ip, src_port, dst_ip, dst_port
                            QList<QString> flow = {"", "", "", ""};
                            flow[0] = xml.attributes().value("src_ip").toString();
                            flow[1] = xml.attributes().value("src_port").toString();
                            flow[2] = xml.attributes().value("dst_ip").toString();
                            flow[3] = xml.attributes().value("dst_port").toString();
                            flows.append(flow);
                        }
                    }
                    // The end of an element.
                    else if(xml.isEndElement()) {
                        if(xml.name() == "anomaly") {
                            // All values within one anomaly tag parsed. Create the
                            // ... table rows for each flow.
                            for(QList<QString> &flow : flows) {
                                // Create a new row in the table of anomalies.
                                QList<QVariant> &row = table_data->newRow();
                                row.append(number);
                                row.append(type);
                                row.append(value);
                                row.append(from);
                                row.append(to);
                                row.append(flow[0]);
                                row.append(flow[1]);
                                row.append(flow[2]);
                                row.append(flow[3]);
                            }
                            // Break out of reading the anomaly tag.
                            break;
                        }
                    }
                }
            }
        }
    }
    if(xml.hasError()) {
        qCritical() << "Errors found while parsing MAWI XML labels.";
        QString error = "Errors found while parsing MAWI XML labels.";
        setLogError(error);
        // Return stating that there was an error.
        return false;
    }

    // No errors, exit with a true status.
    return true;
}
