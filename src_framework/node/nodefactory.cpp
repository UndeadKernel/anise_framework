#include "nodefactory.h"
#include "node.h"
#include "../data/datafactory.h"
#include <dlfcn.h>
#include <QDebug>
#include <QRegExp>
#include "loginfo.h"
#include <QDateTime>

CNodeFactory *CNodeFactory::m_instance = nullptr;


//------------------------------------------------------------------------------
// Constructor and Destructor

CNodeFactory::CNodeFactory()
{

}


//------------------------------------------------------------------------------
// Public Functions

CNodeFactory &CNodeFactory::instance()
{
    if(m_instance == nullptr) {
        m_instance = new CNodeFactory();
    }

    return *m_instance;
}

void CNodeFactory::loadLibraries()
{
    CDynamicFactory::loadLibraries("nodes", "lib*node.so", RTLD_NOW);
}

bool CNodeFactory::nodeAvailable(QString node_class)
{
    return m_makers.contains(node_class);
}

bool CNodeFactory::configTemplate(QString node_class_name, CNodeConfig &config)
{
    if(!m_config_makers.contains(node_class_name)) {
        return false;
    }

    node_configure_fnc configure = m_config_makers.value(node_class_name);
    if(configure == nullptr) {
        return false;
    }

    // Call the configure function of the loaded node.
    configure(config);

    return true;
}

CNode *CNodeFactory::createNode(QString node_class_name, const CNodeConfig &config)
{
    CLogInfo log;

    if(!m_makers.contains(node_class_name)) {
        qCritical() << "The node" << node_class_name
                 << "could not be created.";
#ifdef tadasa
        log.setMsg("The node"+node_class_name+"could not be created.");
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::error);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif

        return nullptr;
    }

    node_maker_fnc make = m_makers.value(node_class_name);
    CNode *node = make(config);

    // Init the Node.
    if(node != nullptr) {
        // Send the DataFactory so that the Node can instantiate data classes.
        node->init(CDataFactory::instance());
    }

    return node;
}

QStringList CNodeFactory::availableNodes()
{
    QStringList node_class_list;

    for(QString node_class : m_config_makers.keys()) {
        node_class_list << node_class;
    }

    return node_class_list;
}

//------------------------------------------------------------------------------
// Protected Functions

void CNodeFactory::addLibrary(void *library_handle, QString filename)
{
    QString name;
    CLogInfo log;

    // Obtain the name of the node.
    QRegExp regexp(".*lib(\\w+)node.so$");
    if(regexp.indexIn(filename) != -1) {
        name = regexp.cap(1);
    }
    else {
        qWarning() << "Could not load the node file " << filename;
#ifdef tadasa
        log.setMsg("Could not load the node file "+filename);
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif


        return;
    }

    // Make sure a node with a similar name has not already been loaded.
    if(m_makers.contains(name)) {
        qWarning() << "The Node Factory already loaded a node called '"
                   << name
                   << "'. Loaded by" << filename;
#ifdef tadasa
        log.setMsg("The Node Factory already loaded a node called '"+name+"'. Loaded by"+filename);
        log.setName("Anise");
        log.setSrc(CLogInfo::ESource::framework);
        log.setStatus(CLogInfo::EStatus::warning);
        log.setTime(QDateTime::currentDateTime());
        log.printMessage();
 #endif



        return;
    }

    // Register the maker of the node.
    m_makers[name] = (node_maker_fnc)dlsym(library_handle, "maker");
    // Register the configurator of the node.
    m_config_makers[name] = (node_configure_fnc)dlsym(library_handle, "configure");
}
