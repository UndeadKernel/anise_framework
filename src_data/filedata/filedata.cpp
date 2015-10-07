#include "filedata.h"

#include <QFile>
#include <QIODevice>

//------------------------------------------------------------------------------
// Constructor and Destructor

CFileData::CFileData()
    : CData()
    , m_bytes()
    , m_binary_data(true)
{

}

CFileData::CFileData(bool binary_data, const QByteArray &bytes)
    : CData()
    , m_bytes(bytes)
    , m_binary_data(binary_data)
{

}


//------------------------------------------------------------------------------
// Public Functions

CDataPointer CFileData::clone() const
{
    CDataPointer clone =
        CDataPointer(new CFileData(m_binary_data, m_bytes));

    return clone;
}

bool CFileData::readFile(QString filename, bool binary)
{
    QFile file(filename);
    m_binary_data = binary;

    QIODevice::OpenMode flags = QIODevice::ReadOnly;
    if(!binary) {
        flags |= QIODevice::Text;
    }

    if(!file.open(flags)) {
        return false;
    }

    // Read the entire file into m_bytes.
    m_bytes = file.readAll();

    return true;
}

bool CFileData::isDataBinary()
{
    return m_binary_data;
}

const QByteArray &CFileData::getBytes() const
{
    return m_bytes;
}

int CFileData::numBytes() const
{
    return m_bytes.length();
}

void CFileData::setByte(int offset, quint8 byte)
{
    m_bytes[offset] = byte;
}


//------------------------------------------------------------------------------
// Private Functions
