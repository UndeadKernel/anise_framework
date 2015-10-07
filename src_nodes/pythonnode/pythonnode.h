#ifndef PYTHONNODE_H
#define PYTHONNODE_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include "datawrapfactory.h"
#include "tabledata/tabledata.h"
#include <QObject>
#include <QString>

class CPythonNode: public CNode
{
    Q_OBJECT

  private:
    // Data Structures

  public:
    // Constructor
    explicit CPythonNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);
  protected:
    // Function called when the simulation is started.
    virtual bool start();
    // Receive data sent by other nodes connected to this node.
    virtual bool data(QString gate_name, const CConstDataPointer &data);

  private:
    // Has the PythonQt library been initialized?
    static bool python_init;
    // The factory wrapper class that translates data structures to python objects.
    static CDataWrapFactory data_wrap_factory;

    // Init the PythonQt library and make some CPP classes available in Python.
    static void initPython();
};

#endif // PYTHONNODE_H
