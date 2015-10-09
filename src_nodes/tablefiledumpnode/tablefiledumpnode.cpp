#include "tablefiledumpnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>


//------------------------------------------------------------------------------
// Constructor and Destructor

CTableFileDumpNode::CTableFileDumpNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CTableFileDumpNode::configure(CNodeConfig &config)
{
    config.setDescription("Write any received table data to a file.");

    //Set the category
    config.setCategory("DataDump");

    // Add parameters
    config.addFilename("output_filename", "Output File", "File to be written.");
    config.addBool("append", "Append the table data",
                   "Append the data to the output file instead of replacing it.",
                   false);
    config.addBool("csv", "Table data in CSM format",
                   "Write the table data with the CSV file format.",
                   false);
    // Add the gates.
    config.addInput("in", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CTableFileDumpNode::start()
{
    return true;
}

bool CTableFileDumpNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);

    if(data->getType() != "table") {
        // Do not process data that is not a table.
        return false;
    }

    // User parameters
    QString filename = getConfig().getParameter("output_filename")->value.toString();
    bool append = getConfig().getParameter("append")->value.toBool();
    bool csv = getConfig().getParameter("csv")->value.toBool();

    // The data we have received interpreted as a table.
    auto table = data.staticCast<const CTableData>();

    // Print the table data into the user-supplied filename.
    if(printTable(table, filename, append, csv)) {
        LOG_INFO(QString("Wrote %1").arg(filename));
    }
    else {
        LOG_WARNING("Could NOT write " + filename);
    }

    return true;
}

bool CTableFileDumpNode::printTable(QSharedPointer<const CTableData> &table,
                                    QString filename, bool append, bool csv)
{
    QFile file(filename);
    QIODevice::OpenMode flags = QIODevice::WriteOnly | QIODevice::Text;

    if(append) {
        flags |= QIODevice::Append;
    } else {
        flags |= QIODevice::Truncate;
    }

    if(!file.open(flags)){
        return false;
    }

    QTextStream out(&file);

    // If a table header was defined, print it.
    if(table->headerSize() != 0) {
        // Print table columns
        out << table->headerSize() << endl;
        // Print table header.
        const QList<QString> &header = table->header();
        qint32 header_size = header.size();
        for(qint32 i = 0; i < header_size; ++i) {
            out << header.at(i);
            if(i != header_size - 1) {
                if(!csv) {
                    out << '\t';
                }
                else {
                    out << ", ";
                }
            }
        }
        out << endl;
    }

    // Print each table row as a line in the file.
    qint32 row_count = table->rowCount();
    qint32 col_count = 0;

    for(qint32 i = 0; i < row_count; ++i) {
        const QList<QVariant> &row = table->getRow(i);
        col_count = row.size();
        for(qint32 j = 0; j < col_count; ++j) {
            out << row.at(j).toString();
            if(j != col_count - 1) {
                if(!csv) {
                    out << '\t';
                }
                else {
                    out << ", ";
                }
            }
        }
        out << endl;
    }

    return true;
}
