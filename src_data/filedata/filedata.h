#ifndef FILEDATA_H
#define FILEDATA_H

#include "data/data.h"
#include <QByteArray>

class CFileData: public CData
{
  public:
    explicit CFileData();
    virtual CDataPointer clone() const;
    bool readFile(QString filename, bool binary);
    bool isDataBinary();
    const QByteArray &getBytes() const;
    int numBytes() const;
    void setByte(int offset, quint8 byte);

  private:
    QByteArray m_bytes;
    bool m_binary_data;

    // Constructor for cloning this object.
    explicit CFileData(bool binary_data, const QByteArray &bytes);
};

#endif // FILEDATA_H
