#ifndef CSVPARSERNODE_H
#define CSVPARSERNODE_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include "csvdumpdata/csvdumpdata.h"
#include "tabledata/tabledata.h"\

#include <QObject>
#include <QString>
#include <QTextStream>

class CCsvparserNode: public CNode
{
    Q_OBJECT

  private:
    // Data Structures

  public:
    // Constructor
    explicit CCsvparserNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);

  protected:
    // Function called when the simulation is started.
    virtual bool start();
    // Receive data sent by other nodes connected to this node.
    virtual bool data(QString gate_name, const CConstDataPointer &data);
    //Parse Data into a *.csv file
    //void parseIntoCSV(const QSharedPointer<const CTableData> &table);

  private:
    bool createDataTable(QSharedPointer<CTableData> &table,
        QTextStream &file_bytes, bool header);

};

#endif // CSVPARSERNODE_H
