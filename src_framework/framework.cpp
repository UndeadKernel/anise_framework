#include "framework.h"
#include "messagehandler.h"
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
    QCommandLineOption machineOption("machine",
        "The output is printed in a more friendly way for the purpose of being parsed.");
    parser.addOption(machineOption);
    // The --nodes option
    QCommandLineOption nodesOption("nodes",
                                   "Print all the nodes the framework recognizes. "
                                   "Exit after printing the nodes.");
    parser.addOption(nodesOption);

    parser.process(*QCoreApplication::instance());


    // Are we printing to the console for the humans or for the machines?
    if(!parser.isSet(machineOption)) {
        // Enable pretty printing with out own custom message writer.
        // QT4:
     // qInstallMsgHandler(customMessageWriterQt4);
        // QT5:
        qInstallMessageHandler(humanMessageWriterQt5);
    }
    else {
        qInstallMessageHandler(machineMessageWriterQt5);
    }


    // Load dynamic nodes and messages into their corresponsing factories.
    // ... The data nodes should be loaded first as the nodes use them.
    CDataFactory::instance().loadLibraries();
    CNodeFactory::instance().loadLibraries();

    // Evaluate the parameters
    if(parser.isSet(nodesOption)) {
        // Only print the nodes and exit.
        printNodes(!parser.isSet(machineOption));
        QCoreApplication::exit(0);
        return;
    }

    // Evaluate the Arguments
    const QStringList args = parser.positionalArguments();
    if(args.size() == 0) {
        // No arguments supplied, this is a usage error at this point.
        qCritical() << "A mesh argument must be specified.";
        parser.showHelp(1);
        return;
    }
    if(args.at(0).isEmpty()) {
        // The mesh argument was not supplied.
        qCritical() << "An invalid mesh has been specified.";
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
    qDebug() << "Starting the simulation" << endl
             << "-----------------------";
    m_mesh.startSimulation();
}

void CFramework::printNodes(bool pretty_print)
{
    QJsonArray json_nodes;
    QStringList node_classes = CNodeFactory::instance().availableNodes();

    for(int i = 0; i < node_classes.size(); ++i) {
        QJsonObject json_node;
        CNodeConfig config;
        CNodeFactory::instance().configTemplate(node_classes.at(i), config);
        json_node["class"] = node_classes.at(i);
        json_node["description"] = config.getDescription();

<<<<<<< HEAD
=======


        const QMap<QString, CNodeConfig::SParameterTemplate> parameters = config.parameter_template_map();
        QJsonObject json_parameters;

        //get types of parameters
        foreach(QString key, parameters.keys())
            json_parameters[key] = parameters[key].value.userType();


>>>>>>> 8c9b5c9... added parameter output to printNodes.
        QJsonArray json_input_gates;
        QJsonArray json_output_gates;
        // Get the input gates as JSON objects.
        const QList<CNodeConfig::SGateTemplate> input_gates = \
                config.getInputTemplates();
        for(int i = 0; i < input_gates.size(); ++i) {
            QJsonObject json_gate;
            json_gate["name"] = input_gates.at(i).name;
            json_gate["type"] = input_gates.at(i).msg_type;
            json_input_gates.append(json_gate);
        }
        // Get the output gates as JSON objects.
        const QList<CNodeConfig::SGateTemplate> output_gates = \
                config.getOutputTemplates();
        for(int i = 0; i < output_gates.size(); ++i) {
            QJsonObject json_gate;
            json_gate["name"] = output_gates.at(i).name;
            json_gate["type"] = output_gates.at(i).msg_type;
            json_output_gates.append(json_gate);
        }

        json_node["input_gates"] = json_input_gates;
        json_node["output_gates"] = json_output_gates;
        json_node["parameters"] = json_parameters;
        json_nodes.append(json_node);
    }

    // Print the JSON representation of all nodes to the console.
    QJsonObject json_container;
    json_container["nodes"] = json_nodes;
    QJsonDocument json_doc(json_container);

    if(pretty_print) {
        qDebug() << "Available Nodes: "
                 << endl << json_doc.toJson(QJsonDocument::Indented);
    }
    else {
        // Info messages disabled normally. Force printing by stating with '@'.
        qDebug() << "@" << json_doc.toJson(QJsonDocument::Compact);
    }
}

void CFramework::onMeshInit(bool success)
{
    if(!success) {
        qCritical() << "Simulation not started as "
                    << "not all nodes started correctly.";
        QCoreApplication::exit(1);
    }
    else {
        // Start the simulation.
        simulateMesh();
    }
}

void CFramework::onMeshFinish()
{
    qDebug() << "Simulation finished" << endl
             << "-----------------------";
    QCoreApplication::exit(0);
}

void CFramework::initMesh(QString mesh)
{
    QFile file(mesh);
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        qCritical() << "The mesh" << mesh << "could not be opened.";
        QCoreApplication::exit(1);
        return;
    }
    QTextStream stream(&file);

    if(m_mesh.parseMesh(stream.readAll())) {
        // Start the nodes in the mesh if we succeeded parcing
        // ... the mesh.
        m_mesh.startNodes();
    }
    else {
        qWarning() << "No simulation was started.";
        QCoreApplication::exit(1);
        return;
    }
}
