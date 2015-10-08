#ifndef TABLEFILEDUMPNODE_H
#define TABLEFILEDUMPNODE_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include "tabledata/tabledata.h"
#include <QObject>
#include <QString>

class CTableFileDumpNode: public CNode
{
    Q_OBJECT

  private:
    // Data Structures

  public:
    // Constructor
    explicit CTableFileDumpNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);

  protected:
    // Function called when the simulation is started.
    virtual bool start();
    // Receive data sent by other nodes connected to this node.
    virtual bool data(QString gate_name, const CConstDataPointer &data);
    // Print the supplied data to a file.
    bool printTable(QSharedPointer<const CTableData> &table,
        QString filename, bool append, bool csv);
};

#endif // TABLEFILEDUMPNODE_H
