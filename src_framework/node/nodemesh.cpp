#include "nodemesh.h"
#include "nodefactory.h"
#include "nodestarttask.h"
#include "../settings.h"
#include "../progressinfo.h"
#include "../../src_common/qt-json/json.h"
#include "../data/datafactory.h"
#include "../data/messagedata.h"
#include <QDebug>
#include <QThreadPool>
#define tadasa
#ifdef tadasa
#include "loginfo.h"
#include <QDateTime>
#endif


//------------------------------------------------------------------------------
// Constructor and Destructor

CNodeMesh::CNodeMesh()
    : m_nodes()
    , m_nodes_waiting(0)
    , m_start_success(true)
    , m_nodes_processing(0)
{

}


//------------------------------------------------------------------------------
// Public Functions

bool CNodeMesh::parseMesh(QString json_str)
{
    CLogInfo log;
    bool success;
    QVariant json_variant = QtJson::parse(json_str, success).toMap();

    if(!success) {
        qCritical() << "Failed to parse mesh file.";
#ifdef tadasa
        log.setMsg("Failed to parse mesh file.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        return false;
    }

    QVariantMap json_map = json_variant.toMap();

    // Create and add all the Nodes to our collection of nodes.
    for(QVariant variant : json_map["nodes"].toList()) {
        QVariantMap map = variant.toMap();
        if(!addNode(map)) {
            return false;
        }
    }

    // Create the links between the nodes.
    for(QVariant variant : json_map["connections"].toList()) {
        QVariantMap conn = variant.toMap();
        if(!addConnection(conn)) {
            return false;
        }
    }

    return true;
}

void CNodeMesh::startNodes()
{
    // Set how many nodes we are going to wait for.
    m_nodes_waiting = m_nodes.size();

    QMap<QString, QSharedPointer<CNode>>::iterator i;
    for(i = m_nodes.begin(); i != m_nodes.end(); ++i) {
        auto node = i.value();
        // Create a runnable task that will start the nodes in parallel.
        CNodeStartTask *start_task = new CNodeStartTask(*(node.data()));
        QObject::connect(start_task, SIGNAL(taskFinished(bool)),
                         this, SLOT(onNodeStarted(bool)));

        // Let the ThreadPool delete start_task after finishing.
        start_task->setAutoDelete(true);
        // Start the nodes in parallel using all available threads.
        QThreadPool::globalInstance()->start(start_task);
    }
}

void CNodeMesh::startSimulation()
{
    CLogInfo log;
    qint8 input_gates = 0;
    // Flag to keep track whether the simulation is started or not.
    bool simulation_started = false;

    // Create the message that will be sent.
    CMessageData *msg =
        static_cast<CMessageData *>(CDataFactory::instance().createData("message"));
    if(msg == nullptr) {
        qCritical() << "Could not create start message.";
#ifdef tadasa
        log.setMsg("Could not create start message.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        emit simulationFinished();
        return;
    }
    msg->setMessage("start");
    QSharedPointer<CData> pmsg = QSharedPointer<CData>(msg);

    // Look for nodes without input gates and send them the start message.
    QMap<QString, QSharedPointer<CNode>>::iterator i;
    for(i = m_nodes.begin(); i != m_nodes.end(); ++i) {
        auto node = i.value();
        input_gates = node->inputGatesSize();
        qDebug().nospace().noquote()
                << "Message before";

        if(input_gates == 0) {
            node->processData("", pmsg);
            qDebug().nospace().noquote()
                    << "Message after";
            simulation_started = true;
        }
    }

    if(!simulation_started) {
        qWarning() << "The simulation was not started because "
                   << "we could not figure out where to start.";
#ifdef tadasa
        log.setMsg("The simulation was not started because we could not figure out where to start.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif
       emit simulationFinished();
    }
}



//------------------------------------------------------------------------------
// Private Functions

bool CNodeMesh::addNode(QVariantMap &node_json)
{
    CLogInfo log;
    CNodeConfig conf;
    bool ok;
    QString node_name;
    QString node_class;
    QString node_desc("");
    QVariant v;

    v = node_json["name"];
    if(v.isValid()) {
        node_name = v.toString();
    }

    v = node_json["class"];
    if(v.isValid()) {
        node_class = v.toString();
    }

    // Optional in the json file.
    v = node_json["description"];
    if(v.isValid()) {
        node_desc = v.toString();
    }

    // Verify that this Node was defined properly.
    if(node_name.isEmpty() || node_class.isEmpty()) {
        qWarning() << "The JSON Node definition did not include class or name.";
#ifdef tadasa
        log.setMsg("The JSON Node definition did not include class or name.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif
        return false;
    }

    // Verify that a Node with the same name does not exist already in the map.
    if(m_nodes.contains(node_name)) {
        qWarning() << "A node with the name" << node_name
                   << "has already been added to the mesh.";
#ifdef tadasa
        QString Msg="A node with the name"+node_name+"has already been added to the mesh.";
        log.setMsg(Msg);
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif
       return false;
    }

    // Verify that the requested node class is available.
    ok = CNodeFactory::instance().nodeAvailable(node_class);
    if(!ok) {
        qCritical() << "Cannot create node" << node_name << "."
                    << "The node class" << node_class
                    << "does not exist.";
#ifdef tadasa
        QString Msg;
        Msg="Cannot create node"+node_name+". The node class "+node_class+" does not exist.";
        log.setMsg(Msg);
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        return false;
    }

    // Get the configuration template of the requested node.
    ok = CNodeFactory::instance().configTemplate(
        node_class, conf);
    if(!ok) {
        // The template for the desired Node class was not found.
        qWarning() << "The node class"
                   << node_name << "failed to set its config template.";
#ifdef tadasa
        QString Msg;
        Msg="The node class "+ node_name+" failed to set its config template.";
        log.setMsg(Msg);
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif
        return false;
    }

    // Set the node name.
    conf.setName(node_name);

    // Add a description if it was supplied.
    if(!node_desc.isEmpty()) {
        conf.setDescription(node_desc);
    }

    // Set the node Parameters.
    for(QVariant p : node_json["params"].toList()) {
        QVariantMap param = p.toMap();
        for(QVariant key : param.keys()) {
            ok = conf.setParameter(key.toString(), param.value(key.toString()));
            if(!ok) {
                qWarning() << "Failed to set the parameter" << key.toString()
                           << "in Node" << node_name << ".";
#ifdef tadasa
                QString Msg;
        Msg="Failed to set the parameter "+key.toString()+ " in Node" + node_name + " .";
        log.setMsg(Msg);
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif
                return false;
            }
        }
    }

    // Create the node we've been asked for.
    CNode *node = CNodeFactory::instance().createNode(node_class, conf);
    if(node == nullptr) {
        qWarning() << "Could not create Node" << node_class << ".";
#ifdef tadasa
        log.setMsg("Could not create Node "+node_class+" .");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        return false;
    }
    m_nodes.insert(node_name, QSharedPointer<CNode>(node));
    // Keep track of the processing status of the node.
    QObject::connect(node, SIGNAL(processing(bool)),
                     this, SLOT(onNodeProcessing(bool)));

    return true;
}

bool CNodeMesh::addConnection(QVariantMap& connections_json)
{
    CLogInfo log;
    QSharedPointer<CNode> src_node;
    QSharedPointer<CNode> dest_node;
    QString src_name;
    QString dest_name;
    QString src_gate;
    QString dest_gate;
    QVariant v;

    v = connections_json["src_node"];
    if(v.isValid()) {
        src_name = v.toString();
    }

    v = connections_json["dest_node"];
    if(v.isValid()) {
        dest_name = v.toString();
    }

    v = connections_json["src_gate"];
    if(v.isValid()) {
        src_gate = v.toString();
    }

    v = connections_json["dest_gate"];
    if(v.isValid()) {
        dest_gate = v.toString();
    }

    // Check that all parameters are there in the json string.
    if(src_name.isEmpty() || src_gate.isEmpty() ||
        dest_name.isEmpty() || dest_gate.isEmpty()) {
        qCritical() << "Some connection parameters are missing. Connection not created.";
#ifdef tadasa
        log.setMsg("Some connection parameters are missing. Connection not created.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        return false;
    }

    // Get the nodes.
    if(m_nodes.contains(src_name)) {
        src_node = QSharedPointer<CNode>(m_nodes[src_name]);
    }
    if(m_nodes.contains(dest_name)) {
        dest_node = QSharedPointer<CNode>(m_nodes[dest_name]);
    }

    // Make sure the referenced nodes exist.
    if(src_node.isNull()) {
        qCritical() << "Connection failed: Source node"
                    << src_name << "was not found.";
#ifdef tadasa
        log.setMsg("Connection failed: Source node "+ src_name + " was not found.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

    }
    if(dest_node.isNull()) {
        qCritical() << "Connection failed: Destination node"
                    << dest_name << "was not found.";
#ifdef tadasa
        log.setMsg("Connection failed: Destination node "+ dest_name + " was not found.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

    }
    if(src_node.isNull() || dest_node.isNull()) {
        return false;
    }

    // Attempt to establish the connection.
    if(!src_node->connect(src_gate, *dest_node, dest_gate)) {
        qCritical() << "Failed to establish a connection.";
#ifdef tadasa
        log.setMsg("Failed to establish a connection.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        return false;
    }

    return true;
}

void CNodeMesh::onNodeStarted(bool success)
{
    // Decrease the nodes that have been started and check if there
    // ... are no more pending nodes waiting to finish starting.
    m_nodes_waiting--;

    if(!success) {
        m_start_success = false;
    }

    // Have we finished starting all nodes?
    if(m_nodes_waiting == 0) {
        // Emit a signal that will inform that we are finished initializing all nodes.
        emit nodesStarted(m_start_success);
    }
}

void CNodeMesh::onNodeProcessing(bool not_idle)
{
//    bool simulation_finished = true;

//    // Check if all the nodes are done.
//    QMap<QString, QSharedPointer<CNode>>::iterator it = m_nodes.begin();
//    while(it != m_nodes.end()) {
//        QSharedPointer<CNode> &node = it.value();
//        if(node->isProcessing()) {
//            simulation_finished = false;
//            break;
//        }
//        ++it;
//    }

//    if(simulation_finished) {
//        emit simulationFinished();
//    }

    if(not_idle) {
        // A node started processing something.
        m_nodes_processing++;
    }
    else {
        m_nodes_processing--;
    }

    if(m_nodes_processing == 0) {
        // There are no more nodes processing.
        emit simulationFinished();
    }
}
