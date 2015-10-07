#include "progressinfo.h"
#include "settings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>


CProgressInfo::CProgressInfo()
    : m_src(ESource::null)
    , m_state(EState::null)
    , m_msg(EMsg::null)
    , m_name("")
{

}

QString CProgressInfo::toJsonString()
{
    QJsonDocument json_doc;
    QJsonObject wrap;
    QJsonObject progress;

    // Source
    QString src = this->srcString();
    progress.insert(QString("source"), src);

    // State
    QString state = this->stateString();
    progress.insert(QString("state"), state);

    // Msg
    QString msg = this->msgString();
    progress.insert(QString("msg"), msg);

    // Name
    QString name = this->name();
    progress.insert(QString("name"), name);

    // Info
    QVariant info;
    if(m_info.isNull()) {
        info = QString("");
    }
    else {
        info = m_info;
    }


    progress.insert(QString("info"), QJsonValue::fromVariant(info));

    wrap.insert(QString("progress"), progress);
    json_doc.setObject(wrap);
    QString json_string = json_doc.toJson(QJsonDocument::Compact);

    return json_string;
}

void CProgressInfo::printProgress()
{
    // Is progress reporting for machines or humans enabled?
    if(CSettings::machine()) {
        // For machines.
        qDebug().nospace().noquote()
                << '@'
                << this->toJsonString();
    }
    else {


        // For humans.
        QString message("");

        // The source
        if(this->src() == ESource::framework) {
            message.append("Framework");
        }
        else {
            message.append("Node \"").
                    append(this->name()).
                    append("\"");
        }

        // The state
        switch(this->state()) {
        case EState::init:
            message.append(" initialization");
            break;
        case EState::processing:
            message.append(" processing");
            break;
        default:
            message.append(" unknown");
        }

        // The msg
        switch(this->msg()) {
        case EMsg::start:
            message.append(" started.");
            break;
        case EMsg::stop:
            message.append(" stopped.");
            break;
        case EMsg::percentage:
            message.append(" progress: ").
                    append(this->info().toString())
                    .append("%");
            break;
        case EMsg::error:
            message.append(" error: ").append(this->info().toString());
            break;
        case EMsg::warning:
            message.append(" warning: ").append(this->info().toString());
            break;
        default:
            message.append(" unknown");
        }

        // Print the message.
        switch(this->msg()) {
        case EMsg::error:
            qCritical().noquote() << message;
            break;
        case EMsg::warning:
            qWarning().noquote() << message;
            break;
        default:
            qDebug().noquote() << message;
        }
    }
}

CProgressInfo::ESource CProgressInfo::src() const
{
    return m_src;
}

QString CProgressInfo::srcString() const
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
        src = "";
    }

    return src;
}

void CProgressInfo::setSrc(ESource src)
{
    m_src = src;
}

CProgressInfo::EState CProgressInfo::state() const
{
    return m_state;
}

QString CProgressInfo::stateString() const
{
    QString state("");
    switch(m_state) {
    case EState::init:
        state = "init";
        break;
    case EState::processing:
        state = "processing";
        break;
    default:
        state = "";
    }

    return state;
}

void CProgressInfo::setState(EState state)
{
    m_state = state;
}

CProgressInfo::EMsg CProgressInfo::msg() const
{
    return m_msg;
}

QString CProgressInfo::msgString() const
{
    QString msg("");
    switch(m_msg) {
    case EMsg::start:
        msg = "start";
        break;
    case EMsg::stop:
        msg = "stop";
        break;
    case EMsg::percentage:
        msg = "percentage";
        break;
    case EMsg::error:
        msg = "error";
        break;
    case EMsg::warning:
        msg = "warning";
        break;
    default:
        msg = "";
    }

    return msg;
}

void CProgressInfo::setMsg(EMsg msg)
{
    m_msg = msg;
}

QVariant CProgressInfo::info() const
{
    return m_info;
}

void CProgressInfo::setInfo(QString info)
{
    m_info = QVariant(info);
}

void CProgressInfo::setInfo(qint32 info)
{
   m_info = QVariant(info);
}

QString CProgressInfo::name() const
{
    if(m_src == ESource::framework) {
        // Ignore name if the source is the framework. The framework
        // ... will always have the same name.
        return QString("anise");
    }
    else {
        return m_name;
    }
}

void CProgressInfo::setName(QString name)
{
    m_name = name;
}

