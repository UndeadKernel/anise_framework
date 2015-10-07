#include "pythonnode.h"
#include "tabledatadecor.h"
#include "data/datafactory.h"
#include "data/messagedata.h"
#include "tabledata/tabledata.h"
#include "tcpstreamsdata/tcpstreamsdata.h"
#include "PythonQt.h"
#include <QDebug>
#include <QFile>
#include <QMetaType>


//------------------------------------------------------------------------------
// Static variables

bool CPythonNode::python_init = false;
CDataWrapFactory CPythonNode::data_wrap_factory;


//------------------------------------------------------------------------------
// Constructor and Destructor

CPythonNode::CPythonNode(const CNodeConfig &config, QObject *parent/* = 0*/)
    : CNode(config, parent)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CPythonNode::configure(CNodeConfig &config)
{
    config.setDescription("Run Python scripts as nodes.");
    config.setCategory("Script");
    // Add parameters
    config.addFilename("input_script", "Python Script", "Python script to execute.");

    // Add the gates.
    config.addInput("in_table", "table");
    config.addInput("in_flow", "tcpstreams");
    config.addOutput("out", "table");
}


//------------------------------------------------------------------------------
// Protected Functions

bool CPythonNode::start()
{
    // Init the PythonQt library.
    initPython();

    // Check if the script exists.
    QString script_name = getConfig().getParameter("input_script")->value.toString();
    QFile script_file(script_name);
    if(!script_file.exists()) {
        qWarning() << "Python script does not exist:"
                   << script_name;
        return false;
    }

    return true;
}

bool CPythonNode::data(QString gate_name, const CConstDataPointer &data)
{
    Q_UNUSED(gate_name);

    if(data->getType() == "table") {
        // Get the table data structured received.
        QSharedPointer<const CTableData> p_table = data.staticCast<const CTableData>();
        // Create a table data structure to forward.
        QSharedPointer<CTableData> result_table = QSharedPointer<CTableData>(
                    static_cast<CTableData *>(createData("table")));

        QString script_name = getConfig().getParameter("input_script")->value.toString();
        // Start a new python context independant of any other.
        PythonQtObjectPtr module = PythonQt::self()->createUniqueModule();
        // Evaluate the user-supplied python script.
        module.evalFile(script_name);
        // Call the 'main' function of the user script with the received table
        // ... and a table that will contatin the results as arguments.
        QVariant result = module.call("main",
            QVariantList() << QVariant::fromValue(p_table.data())
                           << QVariant::fromValue(result_table.data()));
        if(result.toBool()) {
            commit("out", result_table);
        }

        return true;
    }
    else if(data->getType() == "tcpstreams") {
        // The TCP Streams data structure.
        QSharedPointer<const CTcpStreamsData> p_flows = data.staticCast<const CTcpStreamsData>();
        // Create a table data structure to forward.
        QSharedPointer<CTableData> result_table = QSharedPointer<CTableData>(
                    static_cast<CTableData *>(createData("table")));

        QString script_name = getConfig().getParameter("input_script")->value.toString();
        // Start a new python context independant of any other.
        PythonQtObjectPtr module = PythonQt::self()->createUniqueModule();
        // Evaluate the user-supplied python script.
        module.evalFile(script_name);
        // Call the 'main' function of the user script with the received table
        // ... and a table that will contatin the results as arguments.
        QVariant result = module.call("main",
            QVariantList() << QVariant::fromValue(p_flows.data())
                           << QVariant::fromValue(result_table.data()));

        // If the python script returned true, commit the results' table.
        if(result.toBool()) {
            commit("out", result_table);
        }

        return true;
    }

    return false;
}

void CPythonNode::initPython()
{
    if(python_init) {
       return;
    }

    // Start the PythonQt library.
    PythonQt::init(PythonQt::RedirectStdOut);
    // Set our DataFactory to translate data structures to python objects.
    PythonQt::self()->addWrapperFactory(&data_wrap_factory);
    // Register data structures.
    PythonQt::self()->registerCPPClass("CTableData", "", "anise");

    // Print all python output to the console.
    QObject::connect(PythonQt::self(),
        &PythonQt::pythonStdOut,
        [](const QString &s) {
            if(!s.trimmed().isEmpty())
                qDebug().nospace().noquote() << s;
        });
    QObject::connect(PythonQt::self(),
        &PythonQt::pythonStdErr,
        [](const QString &s) {
            if(!s.trimmed().isEmpty())
                qWarning().nospace().noquote() << s;
        });

    python_init = true;
}
