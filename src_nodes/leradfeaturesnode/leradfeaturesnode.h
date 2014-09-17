#ifndef LERADFEATURES_H
#define LERADFEATURES_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include "leradfeaturesdata/leradfeaturesdata.h"
#include <QObject>
#include <QString>

class CLeradFeaturesNode : public CNode
{
  Q_OBJECT

  private:
    QSharedPointer<CLeradFeaturesData> m_leradfeatures;

  public:
    // Constructor
    explicit CLeradFeaturesNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);

  public slots:
    // Receive data sent by other nodes connected to this node.
    virtual void data(QSharedPointer<CData> data);

  protected:
    // Initialize "Data" structures which we would like to use within the Node.
    virtual void init(const CDataFactory &data_factory);
    // Function called when the simulation is started.
    virtual void start();
};

#endif // LERADFEATURES_H
