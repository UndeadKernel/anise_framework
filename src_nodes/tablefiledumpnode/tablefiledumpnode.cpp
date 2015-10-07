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
    config.addFilename("filename", "Output File", "File to be written.");
    config.addBool("append", "Append the table data",
                   "Append the data to the output file instead of replacing it.",
                   false);
    config.addBool("csv_output", "Output the data in CSV file",
                   "Output the data in CSV file",
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
    QString info;
    QString warning;

//qDebug()<<"hello"<<getConfig().getParameter("csv_output")->value.toBool();
    if(data->getType() == "table" && getConfig().getParameter("csv_output")->value.toBool()==false) {
        auto table = data.staticCast<const CTableData>();

        // Get the node properties for the file.
        QString filename = getConfig().getParameter("filename")->value.toString();
        bool append = getConfig().getParameter("append")->value.toBool();

        // Print the table data into the user-supplied filename.
        if(printTable(table, filename, append)) {
            logInfo(getConfig().getName() + " stored in " + filename);
        }
        else {
            logWarning(getConfig().getName() + " could NOT be stored in " + filename);
        }
        return true;

    } else if(getConfig().getParameter("csv_output")->value.toBool()){
        auto table = data.staticCast<const CTableData>();
        QString filename = getConfig().getParameter("filename")->value.toString();
        if(dumpIntoFile(table)){
            info=" dumped table in the csv file "+ filename;
            setLogInfo(info);
        }
        else{
            warning=" could NOT dump Table in the csv file" + filename;
            setLogWarning(warning);
        }
    }
    return false;
}

bool CTableFileDumpNode::printTable(QSharedPointer<const CTableData> &table,
                                    QString filename, bool append)
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
        for(const QString& attr : header) {
            out << attr << '\t';
        }
        out << endl;
    }

    // Print each row into the file.
    qint32 row_count = table->rowCount();
    qint32 col_count = 0;

    for(qint32 i = 0; i < row_count; ++i) {
        const QList<QVariant> &row = table->getRow(i);
        col_count = row.size();
        for(qint32 j = 0; j < col_count; ++j) {
            out << row.at(j).toString();
            if(j != col_count - 1) {
                out << "\t";
            }
        }
        out << endl;
    }

    return true;
}

bool CTableFileDumpNode::dumpIntoFile(const QSharedPointer<const CTableData> &table) {
    qint32 attribute_count = table->colCount();
    qDebug()<< table->colCount();
    qint32 rows = table->rowCount();
    qDebug() << "CSV :: "+attribute_count;
    QString csv_value;
    //Write to a csv file
    QString fileName = getConfig().getParameter("filename")->value.toString();
    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream( &file );
    for(qint32 j = 0; j < rows; ++j)
    {
        const QList<QVariant> &row = table->getRow(j);
        for(qint32 i = 0; i < attribute_count; ++i)
        {
          QString value  = row[i].toString()+";";
          csv_value = csv_value+value;
        }
        qDebug()<<csv_value;
        stream << csv_value<<endl;
        csv_value="";
    }
    file.close();
    return true;
}
