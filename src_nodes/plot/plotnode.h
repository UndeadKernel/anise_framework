#ifndef PLOTNODE_H
#define PLOTNODE_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include <QObject>
#include <QString>

class CPlotNode: public CNode
{
    Q_OBJECT

private:
    // Data Structures

public:
    // Constructor
    explicit CPlotNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);

protected:
    // Function called when the simulation is started.
    virtual bool start();
    // Receive data sent by other nodes connected to this node.
    virtual void data(QString gate_name, const CConstDataPointer &data);
};

#endif // PLOTNODE_H

