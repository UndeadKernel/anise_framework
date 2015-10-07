#ifndef TABLEDATA_H
#define TABLEDATA_H

#include "data/data.h"
#include <QList>
#include <QVariant>


class CTableData: public CData
{

  private:
    // Storage of the actual 'table data'.
    QList<QList<QVariant>> m_table;
    // String representations of the table columns.
    QList<QString> m_header;

  public:
    explicit CTableData();
    CTableData(const CTableData& data);

    void reserveRows(qint32 size) { m_table.reserve(size); }
    qint32 rowCount() const { return m_table.size(); }
    qint32 colCount() const;
    void addHeader(QString attr);
    void addHeader(const QList<QString> &attrs);
    qint32 findHeader(QString attr) const;
    const QList<QString> &header() const;
    qint32 headerSize() const;
    QList<QVariant> &newRow();
    const QList<QVariant> &getRow(int irow) const;
    virtual CDataPointer clone() const;
    const QList<QList<QVariant>> &table() const;

    void sort(qint32 field1);
    void sort(qint32 field1, qint32 field2);
};

Q_DECLARE_METATYPE(CTableData*)
Q_DECLARE_METATYPE(const CTableData*)


#endif // TABLEDATA_H
