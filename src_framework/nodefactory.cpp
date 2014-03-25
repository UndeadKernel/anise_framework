#include "nodefactory.h"
#include "node.h"
#include "datafactory.h"
#include <dlfcn.h>
#include <QDebug>
#include <QRegExp>

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
    CDynamicFactory::loadLibraries("./nodes", "lib*node.so", RTLD_NOW);
}

bool CNodeFactory::configTemplate(QString node_name, CNodeConfig &config)
{
    if(!m_config_makers.contains(node_name)) {
        return false;
    }

    node_configure_fnc configure = m_config_makers.value(node_name);
    configure(config);

    return true;
}

CNode *CNodeFactory::createNode(QString node_name, const CNodeConfig &config)
{
    if(!m_makers.contains(node_name)) {
        qDebug() << "CNodeFactory::createNode() Error: The node" << node_name
                 << "could not be created." << endl;
        return nullptr;
    }

    node_maker_fnc make = m_makers.value(node_name);
    CNode *node = make(config);

    // Init the Node.
    node->init(CDataFactory::instance());

    return node;
}


//------------------------------------------------------------------------------
// Protected Functions

void CNodeFactory::addLibrary(void *library_handle, QString filename)
{
    QString name;

    // Obtain the name of the node.
    QRegExp regexp(".*lib(\\w+)node.so$");
    if(regexp.indexIn(filename) != -1) {
        name = regexp.cap(1);
    }

    qDebug() << "CNodeFactory::addLibrary() Info:"
             << "Loaded Node:" << name << endl;

    // Make sure a node with a similar name has not already been loaded.
    if(m_makers.contains(name)) {
        qDebug() << "CNodeFactory::addLibrary() Warning:"
                 << "The Node Factory already loaded a node called '"
                 << name
                 << "'. Loaded by" << filename << endl;
        return;
    }

    // Register the maker of the node.
    m_makers[name] = (node_maker_fnc)dlsym(library_handle, "maker");
    // Register the configurator of the node.
    m_config_makers[name] = (node_configure_fnc)dlsym(library_handle, "configure");
}
