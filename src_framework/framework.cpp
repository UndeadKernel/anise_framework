#include "framework.h"
#include "messagehandler.h"
#include "settings.h"
#include "progressinfo.h"
#include "node/nodefactory.h"
#include "data/datafactory.h"
#include "node/nodeconfig.h"
#include "node/node.h"
#include "node/nodemesh.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QThreadPool>
#include <QObject>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#define tadasa
#ifdef tadasa
#include"loginfo.h"
#include<QDateTime>
#endif

//------------------------------------------------------------------------------
// Constructor and Destructor


CFramework::CFramework(QObject *parent/*= 0*/)
    : QObject(parent)
    , m_mesh()
{
    // Create the mesh connections.
    QObject::connect(&m_mesh, SIGNAL(nodesStarted(bool)),
                     this, SLOT(onMeshInit(bool)));
    QObject::connect(&m_mesh, SIGNAL(simulationFinished()),
                     this, SLOT(onMeshFinish()));
}


//------------------------------------------------------------------------------
// Public Functions

void CFramework::main()
{
#ifdef tadasa
CLogInfo log;
#endif

    // Parse command line parameters.
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "The ANISE Framework is an environment for the development and execution "
        "of machine learning algorithms. It focuses on speed and efficiency.");
    parser.addHelpOption();
    parser.addVersionOption();

    // Set command line arguments
    // The --mesh argument: Process a user supplied mesh
    parser.addPositionalArgument("mesh",
                                 "A mesh that will be executed by the framework.",
                                 "[mesh]");

    // Set the command line options.
    // The --machine option: Output is meant to be processed by a program
    // ... rather than a human.
    QCommandLineOption machine_option("machine",
        "The output is printed in a more friendly way for the purpose of being parsed.");
    parser.addOption(machine_option);
    // The --nodes option
    QCommandLineOption nodes_option("nodes",
                                   "Print all the nodes the framework recognizes. "
                                   "Exit after printing the nodes.");
    parser.addOption(nodes_option);
    // The --progress option
    QCommandLineOption progress_option("progress",
                                       "Show additional progress information besides the default"
                                       " status messages.");
    parser.addOption(progress_option);
    // The --dbg_function option
    QCommandLineOption dbg_function_option("dbg_function",
        "Show the function that printed a message into the console.");
    parser.addOption(dbg_function_option);

#ifdef tadasa
    QCommandLineOption msglog("msglog",
        "Show the function that prints message log");
    parser.addOption(msglog);
   #endif

    parser.process(*QCoreApplication::instance());


    // Are we printing to the console for the humans or for the machines?
    if(!parser.isSet(machine_option)) {
        CSettings::set("machine", false);
        // Enable pretty printing with out own custom message writer.
        // QT4:
        // qInstallMsgHandler(customMessageWriterQt4);
        // QT5:
        qInstallMessageHandler(humanMessageWriterQt5);
    }
    else {
        CSettings::set("machine", true);
        qInstallMessageHandler(machineMessageWriterQt5);
    }

    CSettings::set("progress", parser.isSet(progress_option));
    CSettings::set("dbg_function", parser.isSet(dbg_function_option));
#ifdef tadasa
    CSettings::set("msglog",parser.isSet(msglog));
#endif
    // Load dynamic nodes and messages into their corresponsing factories.
    // ... The data nodes should be loaded first as the nodes use them.
    CDataFactory::instance().loadLibraries();
    CNodeFactory::instance().loadLibraries();

    if(parser.isSet(nodes_option)) {
        // Only print, the nodes and exit.
        printNodes();
        QCoreApplication::exit(0);
        return;
    }

    // Evaluate the Arguments
    const QStringList args = parser.positionalArguments();
    if(args.size() == 0) {
        // No arguments supplied, this is a usage error at this point.
        qCritical() << "A mesh argument must be specified.";
#ifdef tadasa
        log.setMsg("A mesh argument must be specified.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif
        parser.showHelp(1);
        return;
    }
    if(args.at(0).isEmpty()) {
        // The mesh argument was not supplied.
        qCritical() << "An invalid mesh has been specified.";
#ifdef tadasa
        log.setMsg("An invalid mesh has been specified.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        parser.showHelp(1);
        return;
    }

    // Create the mesh.
    initMesh(args.at(0));
}


//------------------------------------------------------------------------------
// Private Functions

void CFramework::simulateMesh()
{
    CProgressInfo progress;
    progress.setSrc(CProgressInfo::ESource::framework);
    progress.setState(CProgressInfo::EState::processing);
    progress.setMsg(CProgressInfo::EMsg::start);
    progress.printProgress();
    qDebug() << "-----------------------";
    m_mesh.startSimulation();
}

void CFramework::printNodes()
{
    // Get the setting that will tell us if we pretty print or not.
    bool pretty_print = !CSettings::get("machine").toBool();

    QJsonArray json_nodes;
    QStringList node_classes = CNodeFactory::instance().availableNodes();

    for(int i = 0; i < node_classes.size(); ++i) {
        QJsonObject json_node;
        CNodeConfig config;
        CNodeFactory::instance().configTemplate(node_classes.at(i), config);
        json_node["class"] = node_classes.at(i);
        json_node["description"] = config.getDescription();

        // Print the parameters of the node.
        QJsonArray json_parameters;
        QStringList node_parameters = config.getAllParameters();
        for(QString param : node_parameters) {
            QJsonObject json_param;
            const CNodeConfig::SParameterTemplate *param_template =
                    config.getParameter(param);
            json_param["key"] = param;
            json_param["name"] = param_template->name;
            json_param["type"] =
                    QString(QVariant::typeToName(param_template->type));
            json_param["default"] = QJsonValue::fromVariant(
                        param_template->value);
            json_param["description"] = param_template->description;
            json_parameters.append(json_param);
        }
        json_node["parameters"] = json_parameters;

        QJsonArray json_input_gates;
        QJsonArray json_output_gates;
        // Get the input gates as JSON objects.
        const QList<CNodeConfig::SGateTemplate> input_gates =
                config.getInputTemplates();
        for(int i = 0; i < input_gates.size(); ++i) {
            QJsonObject json_gate;
            json_gate["name"] = input_gates.at(i).name;
            json_gate["type"] = input_gates.at(i).msg_type;
            json_input_gates.append(json_gate);
        }
        // Get the output gates as JSON objects.
        const QList<CNodeConfig::SGateTemplate> output_gates =
                config.getOutputTemplates();
        for(int i = 0; i < output_gates.size(); ++i) {
            QJsonObject json_gate;
            json_gate["name"] = output_gates.at(i).name;
            json_gate["type"] = output_gates.at(i).msg_type;
            json_output_gates.append(json_gate);
        }

        json_node["input_gates"] = json_input_gates;
        json_node["output_gates"] = json_output_gates;

        json_nodes.append(json_node);
    }

    // Print the JSON representation of all nodes to the console.
    QJsonObject json_container;
    json_container["nodes"] = json_nodes;
    QJsonDocument json_doc(json_container);

    if(pretty_print) {
        qDebug().nospace().noquote()
                << "Available Nodes: "
                 << endl << json_doc.toJson(QJsonDocument::Indented);
    }
    else {
        // Info messages disabled normally. Force printing by stating with '@'.
        qDebug().nospace().noquote()
                << '@'
                << json_doc.toJson(QJsonDocument::Compact);
    }
}

void CFramework::onMeshInit(bool success)
{
    CProgressInfo progress;
CLogInfo log;
    progress.setSrc(CProgressInfo::ESource::framework);
    progress.setState(CProgressInfo::EState::init);

    if(!success) {
        progress.setMsg(CProgressInfo::EMsg::error);
        progress.setInfo("Simulation not started.");
        progress.printProgress();
#ifdef tadasa
        log.setMsg("Simulation not started.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        QCoreApplication::exit(1);
    }
    else {
        // Start the simulation.
        progress.setMsg(CProgressInfo::EMsg::stop);
        progress.printProgress();
        simulateMesh();
    }
}

void CFramework::onMeshFinish()
{
    CProgressInfo progress;
    progress.setSrc(CProgressInfo::ESource::framework);
    progress.setState(CProgressInfo::EState::processing);
    progress.setMsg(CProgressInfo::EMsg::stop);

    qDebug() << "-----------------------";
    progress.printProgress();

    // Exit the application with no errors.
    QCoreApplication::exit(0);
}

void CFramework::initMesh(QString mesh)
{
    CProgressInfo progress;
    CLogInfo log;
    progress.setSrc(CProgressInfo::ESource::framework);
    progress.setState(CProgressInfo::EState::init);

    QFile file(mesh);
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        // Report an error and exit.
        progress.setMsg(CProgressInfo::EMsg::error);
        progress.setInfo(QString("The mesh '%1' could not be opened.").arg(mesh));
        progress.printProgress();
#ifdef tadasa
        log.setMsg(QString("The mesh '%1' could not be opened.").arg(mesh));
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif
        QCoreApplication::exit(1);
        return;
    }
    QTextStream stream(&file);

    if(m_mesh.parseMesh(stream.readAll())) {
        // Start the nodes in the mesh if we succeeded parsing
        // ... the mesh.
        progress.setMsg(CProgressInfo::EMsg::start);
        progress.printProgress();
        m_mesh.startNodes();
    }
    else {
        progress.setMsg(CProgressInfo::EMsg::warning);
        progress.setInfo("No simulation was started.");
        progress.printProgress();
#ifdef tadasa
        log.setMsg("No simulation was started.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        QCoreApplication::exit(1);
        return;
    }
}
