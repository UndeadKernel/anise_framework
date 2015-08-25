#include "tcpdumpnode.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "filedata/filedata.h"
#include <QDebug>
#include <QFile>

//------------------------------------------------------------------------------
// Constructor and Destructor

CTcpDumpNode::CTcpDumpNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CTcpDumpNode::configure(CNodeConfig &config)
{
    config.setDescription("Parse the contents received into TCP Packets.");

    // Add inputs and outputs
    config.addInput("in", "file");
    config.addOutput("out", "tcpdump");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CTcpDumpNode::start()
{
    return true;
}

bool CTcpDumpNode::data(QString gate_name, const CConstDataPointer &data)
{
    // No need to track gates.
    Q_UNUSED(gate_name);

    if(data->getType() == "file") {
        QSharedPointer<const CFileData> file = data.staticCast<const CFileData>();
        QSharedPointer<CTcpDumpData> tcpdump = QSharedPointer<CTcpDumpData>(
                    static_cast<CTcpDumpData *>(createData("tcpdump")));

        setProgress(0);
        tcpdump->setNodeReporter(this);
        tcpdump->parse(file->getBytes());
        qDebug() << "Packets parsed:"
                 << tcpdump->availablePackets();
        QString info="Packets parsed:"+ QVariant(tcpdump->availablePackets()).toString();
        setLogInfo(info);                 
        tcpdump->unsetNodeReporter();
        setProgress(100);

        commit("out", tcpdump);
        return true;
    }

    return false;
}
