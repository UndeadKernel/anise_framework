#ifndef CSVDUMPNODE_H
#define CSVDUMPNODE_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include "tabledata/tabledata.h"
#include <QObject>
#include <QString>

class CCsvdumpNode: public CNode
{
    Q_OBJECT

private:
    // Data Structures
    QSharedPointer<const CTableData> dump_table;

public:
    // Constructor
    explicit CCsvdumpNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);

protected:
    // Function called when the simulation is started.
    virtual bool start();
    // Receive data sent by other nodes connected to this node.
    virtual bool data(QString gate_name, const CConstDataPointer &data);
    //Dump Data into a *.csv file
    void dumpIntoFile(const QSharedPointer<const CTableData> &table);
};

#endif // CSVDUMPNODE_H

