#include "csvparsernode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "csvdumpdata/csvdumpdata.h"
#include "tabledata/tabledata.h"
#include "filedata/filedata.h"
#include <QDebug>
#include <QFile>
#include <QSharedPointer>


//------------------------------------------------------------------------------
// Constructor and Destructor

CCsvparserNode::CCsvparserNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CCsvparserNode::configure(CNodeConfig &config)
{
    // Set a Description of this node.
    config.setDescription("Parse CSV files as tables.");
    // Add parameters
    config.addBool("headers", "Headers Included",
        "Are the headers included in the CSV file?", false);
    config.setCategory("Parser");
    // Add the gates.
    config.addInput("in", "file");
    config.addOutput("out", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CCsvparserNode::start()
{
    return true;
}

bool CCsvparserNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);

    if(data->getType() != "file") {
        // We only parse file data types. Ignore all other things.
        return false;
    }

    // Interpret the received data as a file and get the bytes of the file.
    QSharedPointer<const CFileData> file = data.staticCast<const CFileData>();
    QTextStream file_bytes(file->getBytes());

    // Have we received a text file?
    if(file->isDataBinary()) {
        commitError("out", "Binary data instead of text data received.");
        return true;
    }

    // Get the user parameters.
    bool headers = getConfig().getParameter("headers")->value.toBool();

    // Create a table that will contain
    QSharedPointer<CTableData> csv_table;
    if(!createDataTable(csv_table, file_bytes, headers)) {
        commitError("out", "Could not create table.");
        return true;
    }

    // Parse each line of the file.
    while(!file_bytes.atEnd()) {
        QString line = file_bytes.readLine();
        QStringList entries = line.split(',');
        QList<QVariant> &row = csv_table->newRow();
        for(int i = 0; i < entries.length(); ++i) {
            row.append(entries.at(i));
        }
    }

    commit("out", csv_table);
    return true;
}

bool CCsvparserNode::createDataTable(
    QSharedPointer<CTableData> &table,
    QTextStream &file_bytes,
    bool headers)
{
    table = autoCreateData<CTableData>("table");

    // Set the headers if there are any.
    if(headers) {
        if(!file_bytes.atEnd()) {
            QString line = file_bytes.readLine();
            QStringList entries = line.split(',');
            table->addHeader(entries);
        } else {
            // Could not parse headers.
            return false;
        }
    }

    return true;
}
