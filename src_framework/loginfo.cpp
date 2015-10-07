#include "loginfo.h"
#include "settings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDateTime>

CLogInfo::CLogInfo()
    : m_src(ESource::null)
    , m_status(EStatus::null)
    , m_src_name("")
    , m_msg("")
    , m_time()
{

}

QString CLogInfo::toJsonString()
{
    QJsonDocument json_doc;
    QJsonObject wrap;
    QJsonObject log;

    // Status
    QString status = statusString();
    log.insert(QString("status"), status);

    // Source
    QString src;
    if(m_src == ESource::null) {
        src = "framework";
    } else {
        src = srcString();
    }
    log.insert(QString("source"), src);

    // Name
    QString name = getSrcName();
    log.insert(QString("src_name"), name);

    // Msg
    log.insert(QString("msg"), m_msg);

    // Time
    QDateTime time;
    if(m_time.isNull()) {
        time = QDateTime::currentDateTime();
    }
    else {
        time = m_time;
    }
    log.insert(QString("time"), time.toString());

    wrap.insert(QString("log"), log);
    json_doc.setObject(wrap);
    QString json_string = json_doc.toJson(QJsonDocument::Compact);

    return json_string;
}

void CLogInfo::printMessage()
{
    // Is progress reporting for machines or humans enabled?
    if(CSettings::machine()) {
        // For machines.
        qDebug().nospace().noquote()
                << '@'
                << toJsonString();
    }
    else {
        // For humans.
        QString message("");

        // The source
        if(this->src() == ESource::framework) {
            message.append("Framework: ");
        }
        else {
            message.append("Node \"").append(getSrcName()).append("\": ");
        }

        // The msg
        message.append(msg());

        // Print the message.
        switch(this->status()) {
          case EStatus::error:
            qCritical().noquote() << message;
            break;
          case EStatus::warning:
            qWarning().noquote() << message;
            break;
          default:
            qDebug().noquote() << message;
        }
    }
}

CLogInfo::ESource CLogInfo::src() const
{
    return m_src;
}

QString CLogInfo::srcString() const
{
    QString src("");
    switch(m_src) {
      case ESource::node:
        src = "node";
        break;
      case ESource::framework:
        src = "framework";
        break;
      default:
        src = "unknown";
    }

    return src;
}

void CLogInfo::setSrc(ESource src)
{
    m_src = src;
}

CLogInfo::EStatus CLogInfo::status() const
{
    return m_status;
}

QString CLogInfo::statusString() const
{
    QString status("");
    switch(m_status) {
      case EStatus::info:
        status = "info";
        break;
      case EStatus::error:
        status = "error";
        break;
      case EStatus::warning:
        status = "warning";
        break;
      default:
        status = "unknown";
    }

    return status;
}

void CLogInfo::setStatus(EStatus status)
{
    m_status = status;
}

QString CLogInfo::msg() const
{
    return m_msg;
}

void CLogInfo::setMsg(QString msg)
{
    m_msg = msg;
}

QString CLogInfo::getSrcName() const
{
    if(m_src == ESource::framework) {
        // Ignore name if the source is the framework. The framework
        // ... will always have the same name.
        return QString("anise");
    }
    else {
        return m_src_name;
    }
}

void CLogInfo::setName(QString name)
{
    m_src_name = name;
}

void CLogInfo::setTime(QDateTime time)
{
    m_time = time;
}
