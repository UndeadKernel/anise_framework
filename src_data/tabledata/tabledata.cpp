#include "tabledata.h"
#include <QDebug>


//------------------------------------------------------------------------------
// Constructor and Destructor

CTableData::CTableData()
    : CData()
{

}

CTableData::CTableData(const CTableData &data)
{
    Q_UNUSED(data);
    qDebug() << "CTableData copy Constructor called.";
}


//------------------------------------------------------------------------------
// Public Functions

qint32 CTableData::colCount() const
{
    if(m_table.size() > 0) {
        return m_table[0].size();
    }

    return 0;
}

void CTableData::addHeader(QString attr)
{
    m_header.append(attr);
}

void CTableData::addHeader(const QList<QString> &attrs)
{
    for(const QString &attr : attrs) {
        m_header.append(attr);
    }
}

// Search the headers for 'attr' and return its index or -1 if not found.
qint32 CTableData::findHeader(QString attr) const
{
    return m_header.indexOf(attr);
}

const QList<QString> &CTableData::header() const
{
    return m_header;
}

qint32 CTableData::headerSize() const
{
    return m_header.size();
}

QList<QVariant> &CTableData::newRow()
{
    m_table.append(QList<QVariant>());
    m_table.last().reserve(headerSize());

    return m_table.last();
}

const QList<QVariant> &CTableData::getRow(int i_row) const
{
    return m_table.at(i_row);
}

CDataPointer CTableData::clone() const
{
    qCritical() << "Cloning not implemented.";

    return CDataPointer();
}

const QList<QList<QVariant>> &CTableData::table() const
{
    return m_table;
}

void CTableData::sort(qint32 field1)
{
    // The lambda sort function
    auto f_less_than = [&] (const QList<QVariant> &r1, const QList<QVariant> &r2)
    {
        if(r1.at(field1) < r2.at(field1)) {
            return true;
        } else {
            return false;
        }
    };
    std::sort(m_table.begin(), m_table.end(), f_less_than);
}

void CTableData::sort(qint32 field1, qint32 field2)
{
    // Lambda sort function.
    auto f_less_than = [&] (const QList<QVariant> &r1, const QList<QVariant> &r2)
    {
        if(r1.at(field1).toString() < r2.at(field1).toString()) {
            return true;
        }
        else if(r1.at(field1).toString() == r2.at(field1).toString()) {
            if(r1.at(field2).toString() < r2.at(field2).toString()) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    };
    std::sort(m_table.begin(), m_table.end(), f_less_than);
}


//------------------------------------------------------------------------------
// Private Functions
