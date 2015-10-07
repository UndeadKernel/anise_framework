#include "csvparsernode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "csvdumpdata/csvdumpdata.h"
#include "tabledata/tabledata.h"
#include <QDebug>
#include <QFile>


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
    config.setDescription("Parses CSV files into tables");
    // Add parameters
    config.addFilename("file", "Input File", "CSV File to be read from disk.");
    config.setCategory("Parser");
    // Add the gates.
    //config.addInput("in", "table");
    config.addOutput("out", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CCsvparserNode::start()
{
    QVariant filename = getConfig().getParameter("file")->value;
    // Check if the user supplied file exists before we start processing.
    QFile file(filename.toString());
    if(!file.exists()) {
        QString error="File" + filename.toString() + " does not exist.";
        qCritical() << error;
        setLogError(error);
        return false;
    }
    return createDataTable();
}

bool CCsvparserNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);
     if(data->getType() == "message") {
         auto pmsg = data.staticCast<const CMessageData>();
         QString msg = pmsg->getMessage();
         if(msg == "start") {
               QVariant filename = getConfig().getParameter("file")->value;               
               QFile file(filename.toString());
               file.open(QIODevice::ReadOnly | QIODevice::Text);
               //qDebug() << "File opened successfully";
               QTextStream stream( &file );
               QString line = stream.readAll();
               QRegExp rx("(\\n)");//Regualr expressions for semi-colon and next line //|\\,\\;|
               QStringList lines = line.split(rx,QString::SkipEmptyParts);
               qint32 lineSize = lines.size();
               parser_table->reserveRows(parser_table->rowCount()+lineSize);

               // Optimize the row allocation space for the table.
               for (int i = 0; i<lines.size(); ++i)
               {
                   extractFeatures(lines[i]);
               }

                commit("out", parser_table);
                parser_table.clear();
                //qDebug() << "data commited";
         }
         return true;
     }
     return false;
}

bool CCsvparserNode::createDataTable()
{
    parser_table = QSharedPointer<CTableData>(
                static_cast<CTableData *>(createData("table")));

    if(!parser_table.isNull()) {
        parser_table->addHeader("x-axis");
        parser_table->addHeader("y-axis");
        return true;
    } else {
        return false;
    }
}

void CCsvparserNode::extractFeatures(const QString &line)
{
    QList<QVariant> &row = parser_table->newRow();
    //row<<line;
    QRegExp rx("(\\,|\\;)");//Regualr expressions for semi-colon and next line //|\\,\\;|
    QStringList dataPoints = line.split(rx,QString::SkipEmptyParts);
    //QStringList dataPoints =  line.split(",");
    for (qint32 i = 0 ; i<dataPoints.size(); i++)
    {
        row<<dataPoints[i];
    }
}
