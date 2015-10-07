#include "csvdumpdata.h"
#include <QDebug>


//------------------------------------------------------------------------------
// Constructor and Destructor

CCsvdumpData::CCsvdumpData()
    : CData()
{

}

CCsvdumpData::CCsvdumpData(const CCsvdumpData &data)
{
    Q_UNUSED(data);
    qDebug() << "CCsvdumpData copy Constructor called.";
}


//------------------------------------------------------------------------------
// Public Functions

/*qint32 CCsvdumpData::colCount() const
{
    if(csv_table.size() > 0) {
        return csv_table[0].size();
    }

    return 0;
}

void CCsvdumpData::addHeader(QString attr)
{
    m_header.append(attr);
}

void CCsvdumpData::addHeader(const QList<QString> &attrs)
{
    for(const QString &attr : attrs) {
        m_header.append(attr);
    }
}

// Search the headers for 'attr' and return its index or -1 if not found.
qint32 CCsvdumpData::findHeader(QString attr) const
{
    return m_header.indexOf(attr);
}

const QList<QString> &CCsvdumpData::header() const
{
    return m_header;
}

qint32 CCsvdumpData::headerSize() const
{
    return m_header.size();
}*/

QList<QVariant> &CCsvdumpData::newRow()
{
    csv_table.append(QList<QVariant>());
    //csv_table.last().reserve(headerSize());

    return csv_table.last();
}

const QList<QVariant> &CCsvdumpData::getRow(int i_row) const
{
    return csv_table.at(i_row);
}

CDataPointer CCsvdumpData::clone() const
{
    qCritical() << "Cloning not implemented.";

    return CDataPointer();
}

const QList<QList<QVariant>> &CCsvdumpData::table() const
{
    return csv_table;
}

/*void CCsvdumpData::sort(qint32 field1, qint32 field2)
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
    std::sort(csv_table.begin(), csv_table.end(), f_less_than);
}*/


//------------------------------------------------------------------------------
// Private Functions
