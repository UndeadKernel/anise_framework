#include "node.h"
#include "../data/datafactory.h"
#include "data/errordata.h"
#include "nodegatetask.h"
#include <QDebug>
#include <QCoreApplication>
#include <QMutex>
#include <QPair>
#include <QThreadPool>


//------------------------------------------------------------------------------
// Static variables.
bool CNode::log_progress = false;
bool CNode::log_progress_verbose = true;


//------------------------------------------------------------------------------
// Constructor and Destructor

CNode::CNode(const CNodeConfig &config, QObject *parent/*= 0*/)
 : QObject(parent)
 , m_input_gates()
 , m_output_gates()
 , m_config(config)
 , m_data_factory(nullptr)
 , m_processing(false)
 , m_processing_queue()
 , m_commit_list()
 , m_allow_commit(false)
{
    // Create the gates and gate boxes of this node.
    setupGates(config);
}

CNode::~CNode()
{
    qDebug() << getConfig().getName()
             << "destroyed.";
}


//------------------------------------------------------------------------------
// Public Functions

const CNodeConfig& CNode::getConfig() const
{
    return m_config;
}

bool CNode::connect(QString output_name, const CNode &target, QString input_name)
{
    auto src_gate = findOutputGate(output_name);
    if(src_gate.isNull()) {
        qWarning() << "Could not find gate"
                   << output_name << "in output Node" << m_config.getName() << ".";
        return false;
    }

    auto dest_gate = target.findInputGate(input_name);
    if(dest_gate.isNull()) {
        qWarning() << "Could not find gate"
                   << input_name << "in input Node"
                   << target.m_config.getName() << ".";
        return false;
    }

    // Link the output of the src gate with the dest gate of another node.
    if(!src_gate->link(dest_gate)) {
        qWarning() << "Could not connect nodes."
                   << "(" << m_config.getName() << ") -> ("
                   << target.m_config.getName() << ").";
        return false;
    }
    return true;
}

int CNode::inputGatesSize() const
{
    return m_input_gates.size();
}

int CNode::outputGatesSize() const
{
    return m_output_gates.size();
}

int CNode::inputLinkCount(QString gate_name) const
{
    auto gate = findInputGate(gate_name);
    if(gate.isNull()) {
        return 0;
    }
    else {
        return gate->inputLinks();
    }
}

QString CNode::inputGateName(qint8 index) const
{
    if(index >= inputGatesSize()) {
      return "";
    }

    return m_input_gates.at(index)->name();
}

void CNode::processData(QString gate_name, const CConstDataPointer &data)
{
    // Can we process this message now or should we keep it in a queue for later
    // ... processing?
    if(isProcessing()) {
        qDebug() << "The node"
                 << m_config.getName()
                 << "is queuing the data type"
                 << data->getType();
        // Store the name of the gate and the data it is sending in the queue.
        QPair<QString, CConstDataPointer> gate_and_data;
        gate_and_data.first = gate_name;
        gate_and_data.second = data;
        m_processing_queue.enqueue(gate_and_data);
    }
    else {
        // Set the node as processing something.
        setProcessing(true);
        // Setup and start the task in another thread.
        startGateTask(gate_name, data);
    }
}

bool CNode::isProcessing() const
{
    return m_processing;
}

void CNode::enableProgressReporting(bool enable, bool verbose)
{
    log_progress = enable;
    log_progress_verbose = verbose;
}


//------------------------------------------------------------------------------
// Protected Functions

CData *CNode::createData(QString data_name)
{
    if(m_data_factory == nullptr) {
        qCritical() << "Did you forget to call init() on a Node?";
        return nullptr;
    }

    return m_data_factory->createData(data_name);
}

void CNode::commit(QString gate_name, const CConstDataPointer &data)
{
    if(!m_allow_commit) {
        qWarning() << "Data can only be commited"
                   << "inside the 'data' function of a node.";
        return;
    }

    // Build the commit with the output gate and the data to send.
    QPair<QString, CConstDataPointer> gate_and_data;
    gate_and_data.first = gate_name;
    gate_and_data.second = data;

    // Add the pair to the list of commits to process.
    m_commit_list.append(gate_and_data);
}

void CNode::commitError(QString gate_name, QString error_msg)
{
    if(!m_allow_commit) {
        qWarning() << "Data can only be commited"
                   << "inside the 'data' function of a node.";
        return;
    }

    CErrorData *error =
        static_cast<CErrorData *>(CDataFactory::instance().createData("error"));

    if(error == nullptr) {
        qCritical() << "Could not create CErrorData.";
        return;
    }

    error->setMessage(error_msg);
    QSharedPointer<CData> perror = QSharedPointer<CData>(error);

    // Create the pair to add to the commit list.
    QPair<QString, CConstDataPointer> gate_and_data;
    gate_and_data.first = gate_name;
    gate_and_data.second = perror;
    m_commit_list.append(gate_and_data);
}

qint32 CNode::getInputCount(QString gate_name)
{
    QSharedPointer<CGate> gate = findInputGate(gate_name);
    if(!gate.isNull()) {
        return gate->inputLinks();
    }
    else {
        return 0;
    }
}

void CNode::setProgress(qint8 percentage)
{
    if(!log_progress) {
        return;
    }

    if(log_progress_verbose) {
     qDebug().nospace() << "Node progress: "
                        << getConfig().getName() << " - "
                        << percentage << '%';
    }
    else {
    // Print progress messages in machine and human printing mode
    // ... (by adding an '@' to the beginning of the message).
    qDebug() << '@'
             << "{\"status\":" << "{\"node\":"
             << getConfig().getName() << ", "
             << "\"progress\":" << percentage
             << "}" << "}";
    }
}


//------------------------------------------------------------------------------
// Private Functions

void CNode::init(CDataFactory &data_factory)
{
    // Store a pointer to the data factory. Derived nodes will be able to create
    // ... data structures through this member variable.
    m_data_factory = &data_factory;
}

void CNode::setupGates(const CNodeConfig &config)
{
    // Iterate the input templates to create one gate per input.
    auto input_templates = config.getInputTemplates();
    for(auto it = input_templates.begin(); it != input_templates.end(); ++it) {
        CGate *gate = new CGate(it->name, it->msg_type);
        // Link the gate to this node so that whenever input goes into the gate
        // ... this same node will be the responsible for processing the input.
        gate->link(this);
        m_input_gates.append(QSharedPointer<CGate>(gate));
    }

    // Iterate over the output templates to create one gate per output.
    auto output_templates = config.getOutputTemplates();
    for(auto it = output_templates.begin(); it != output_templates.end(); ++it) {
        CGate *gate = new CGate(it->name, it->msg_type);
        m_output_gates.append(QSharedPointer<CGate>(gate));
    }
}

QSharedPointer<CGate> CNode::findInputGate(QString name) const
{
    // Use C++11 for range loops.
    for(auto gate : m_input_gates) {
        if(gate->name() == name) {
            // Connect the node with the other node.
            return gate;
        }
    }

    // Nothing found, return a null pointer.
    return QSharedPointer<CGate>();
}

QSharedPointer<CGate> CNode::findOutputGate(QString name) const
{
    // Use C++11 for range loops.
    for(auto gate : m_output_gates) {
        if(gate->name() == name) {
            // Connect the node with the other node.
            return gate;
        }
    }

    // Nothing found, return a null pointer.
    return QSharedPointer<CGate>();
}

void CNode::startGateTask(QString gate_name, const CConstDataPointer &data)
{
    // Create a NodeTask.
    CNodeGateTask *node_task = new CNodeGateTask(*this, gate_name, data);
    QObject::connect(node_task, SIGNAL(taskFinished()),
        this, SLOT(onTaskFinished()));

    // Queue the task in the global QThreadPool. It becomes the owner of the
    // ... task.
    // Delete the node_task when it's finished.
    node_task->setAutoDelete(true);
    // Send the task to the ThreadPool.
    QThreadPool::globalInstance()->start(node_task);

}

void CNode::setProcessing(bool proc)
{
    m_processing = proc;

    emit processing(proc);
}

void CNode::onTaskFinished()
{
    // Process each pair in the commit list.
    while(m_commit_list.size() != 0) {
        QPair<QString, CConstDataPointer> pair = m_commit_list.takeFirst();

        auto output_gate = findOutputGate(pair.first);
        if(output_gate.isNull()) {
            qWarning() << "Could not commit data from within"
                       << "Node" << m_config.getName() << ". The gate" << pair.first
                       << "was not found.";
        }
        else {
            output_gate->inputData(pair.second);
        }
    }

    // Process one pending data structure in the processing queue.
    if(m_processing_queue.size() > 0) {
        // Process the top most element.
        QPair<QString, CConstDataPointer> gate_and_data =
                m_processing_queue.dequeue();
        startGateTask(gate_and_data.first, gate_and_data.second);
    }
    else {
        // We are done processing, mark the status flag as such.
        setProcessing(false);
    }
}
