#ifndef CSVDUMPDATA_H
#define CSVDUMPDATA_H

#include "data/data.h"
#include <QList>
#include <QVariant>


class CCsvdumpData: public CData
{

  private:
    // Storage of the actual 'csvdump data'.
    QList<QList<QVariant>> csv_table;
    // String representations of the table columns.
    //QList<QString> m_header;

  public:
    explicit CCsvdumpData();
    CCsvdumpData(const CCsvdumpData& data);

    void reserveRows(qint32 size) { csv_table.reserve(size); }
    qint32 rowCount() const { return csv_table.size(); }
    //qint32 colCount() const;
    //void addHeader(QString attr);
    //void addHeader(const QList<QString> &attrs);
    //qint32 findHeader(QString attr) const;
    //const QList<QString> &header() const;
    //qint32 headerSize() const;
    QList<QVariant> &newRow();
    const QList<QVariant> &getRow(int irow) const;
    virtual CDataPointer clone() const;
    const QList<QList<QVariant>> &table() const;

    //void sort(qint32 field1, qint32 field2);
};

Q_DECLARE_METATYPE(CCsvdumpData*)
Q_DECLARE_METATYPE(const CCsvdumpData*)


#endif // CSVDUMPDATA_H
