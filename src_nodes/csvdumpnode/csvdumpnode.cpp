#include "csvdumpnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include <QDebug>
#include <QFile>


//------------------------------------------------------------------------------
// Constructor and Destructor

CCsvdumpNode::CCsvdumpNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}

//------------------------------------------------------------------------------
// Public Functions

void CCsvdumpNode::configure(CNodeConfig &config)
{
    // Set a Description of this node.
    config.setDescription("Dumps the data into a csv File");
    //Set the category
    config.setCategory("DataDump");
    // Add the gates.
    config.addInput("in", "table");
    //config.addOutput("out", "misc");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CCsvdumpNode::start()
{
    return true;
}

bool CCsvdumpNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);
    // Process framework messages.
    if(data->getType() == "table") {
        dump_table = data.staticCast<const CTableData>();
        dumpIntoFile(dump_table);
        return true;
    }
    return false;
}

void CCsvdumpNode::dumpIntoFile(const QSharedPointer<const CTableData> &table) {

    qint32 attribute_count = table->colCount();
    qDebug()<< table->colCount();
    qint32 rows = table->rowCount();
    qDebug() << "CSV :: "+attribute_count;
    QString csv_value;
    //Write to a csv file   
    QString fileName = "table_data.csv";
    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream( &file );

    for(qint32 j = 0; j < rows; ++j)
    {
        const QList<QVariant> &row = dump_table->getRow(j);
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
}
