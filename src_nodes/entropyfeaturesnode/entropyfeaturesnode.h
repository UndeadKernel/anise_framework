#ifndef ENTROPYFEATURESNODE_H
#define ENTROPYFEATURESNODE_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include "tabledata/tabledata.h"
#include <QObject>
#include <QString>

class CEntropyfeaturesNode: public CNode
{
    Q_OBJECT

  private:
    // Data Structures

  public:
    // Constructor
    explicit CEntropyfeaturesNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);

  protected:
    // Function called when the simulation is started.
    virtual bool start();
    // Receive data sent by other nodes connected to this node.
    virtual bool data(QString gate_name, const CConstDataPointer &data);
    quint64 findLastEndTime(const QSharedPointer<const CTableData> &flows_table,
        quint32 time_end_attr);
    quint64 findFirstEndTime(const QSharedPointer<const CTableData> &flows_table,
        quint32 time_start_attr,
        quint32 time_end_attr);
};

uint qHash(const QVariant &var);

#endif // ENTROPYFEATURESNODE_H
