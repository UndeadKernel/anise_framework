#ifndef DATA_H
#define DATA_H

#include <QString>
#include <QSharedPointer>
#include <QMetaType>
#include <QSharedData>

class CDataFactory;

class CData
{
    friend class CDataFactory;

  private:
    QString m_type_name;

  public:
    // Get the type name of this datatype. Setup when instatiated by the data factory.
    QString getType() const;

  protected:
    // Data objects should be created with the Data Factory class.
    CData();

  private:
    void setTypeName(QString type_name);
};

// Register CData objects so that they can be sent through queued connections.
Q_DECLARE_METATYPE(QSharedPointer<CData>)

#endif // DATA_H
