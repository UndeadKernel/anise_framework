#include "filenode.h"
#include "data/datafactory.h"
#include "tabledata/tabledata.h"
#include "data/messagedata.h"
#include <QDebug>
#include <QFile>

//------------------------------------------------------------------------------
// Constructor and Destructor

CFileNode::CFileNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CFileNode::configure(CNodeConfig &config)
{
    // Add parameters
    config.addFilename("file", "Input File",
                       "Path of the file to read from disk.");
    config.addBool("binary", "Binary format",
                   "Parse the file contents as binary data.");

    // Add inputs and outputs
    config.addOutput("out", "file");
}

//------------------------------------------------------------------------------
// Protected Functions

bool CFileNode::start()
{
    QVariant filename = getConfig().getParameter("file")->value;

    // Check if the user supplied file exists before we start processing.
    QFile file(filename.toString());
    if(!file.exists()) {
        qCritical() << "File" << filename.toString() << "does not exist.";
        QString error="File" + filename.toString() + "does not exist.";
        setLogError(error);
        return false;
    }

    return true;
}

bool CFileNode::data(QString gate_name, const CConstDataPointer &data)
{
    // No input gates.
    Q_UNUSED(gate_name);

    if(data->getType() == "message") {
        auto pmsg = data.staticCast<const CMessageData>();
        QString msg = pmsg->getMessage();
        if(msg == "start") {

            // Create and read the file.
            QSharedPointer<CFileData> file_data =
                    QSharedPointer<CFileData>(
                        static_cast<CFileData *>(createData("file")));

            QVariant filename = getConfig().getParameter("file")->value;
            QVariant binary = getConfig().getParameter("binary")->value;

            if(!file_data.isNull() &&
                    file_data->readFile(filename.toString(),
                                        binary.toBool())) {
                commit("out", file_data);
            }
            else {
                commitError("out", "Could not read file.");
            }
            return true;
        }
    }

    return false;
}
