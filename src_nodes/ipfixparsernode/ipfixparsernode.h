#ifndef IPFIXPARSERNODE_H
#define IPFIXPARSERNODE_H

#include "node/node.h"
#include "node/nodeconfig.h"
#include "tabledata/tabledata.h"
#include <QObject>
#include <QString>
#include <fixbuf/public.h>

class CIpfixparserNode: public CNode
{
    Q_OBJECT

  private:
    // Data Structure
    QSharedPointer<CTableData> m_table;

    // IPFIX related variables
    fbInfoModel_t *m_info_model;
    fbSession_t *m_session;
    fBuf_t *m_buf;
    fbTemplate_t *m_default_template;
    fbTemplate_t *m_stats_template;
    quint16 m_flows_template_id;
    quint16 m_stats_template_id;

  public:
    // Constructor
    explicit CIpfixparserNode(const CNodeConfig &config, QObject *parent = 0);
    // Set the configuration template for this Node.
    static void configure(CNodeConfig &config);

  protected:
    // Function called when the simulation is started.
    virtual bool start();
    // Receive data sent by other nodes connected to this node.
    virtual bool data(QString gate_name, const CConstDataPointer &data);
    // Initialize tables of specifications.
    void initSpecsTable(QSharedPointer<CTableData> flows_table, fbInfoElementSpec_t specs[]);
    // Convert a numerical IP6 into a string.
    QString ipv6ToString(quint8 *ip_addr);
};

#endif // IPFIXPARSERNODE_H
