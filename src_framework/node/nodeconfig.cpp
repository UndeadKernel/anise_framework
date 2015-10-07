#include "nodeconfig.h"
#include "../loginfo.h"
#include <QDebug>


//------------------------------------------------------------------------------
// Constructor and Destructor

CNodeConfig::CNodeConfig()
    : m_name("")
    , m_description("")
{

}


//------------------------------------------------------------------------------
// Public Functions

void CNodeConfig::setName(QString name)
{
    m_name = name;
}

const QString CNodeConfig::getName() const
{
    return m_name;
}

void CNodeConfig::setDescription(QString description)
{
    m_description = description;
}

const QString CNodeConfig::getDescription() const
{
    return m_description;
}

QString CNodeConfig::getCategory() const
{
    return m_category;
}

void CNodeConfig::setCategory(const QString &value)
{
    m_category = value;
}

bool CNodeConfig::setParameter(QString key, QVariant value) const
{
    // Key exists?
    if(!m_parameter_template_map.contains(key)) {
        CLogInfo log;
        log.setMsg(QString("The parameter %1 has not been defined"
                "in the configuration template.").arg(key));
        log.setSrc(CLogInfo::ESource::node);
        log.setName(m_name);
        log.setStatus(CLogInfo::EStatus::warning);
        log.printMessage();
        return false;
    }

    SParameterTemplate &param_template = m_parameter_template_map[key];

    // Verify that the supplied value matches the required value.
    if(value.type() != param_template.type) {
        // Allow conversions from uint to int, but not the other way around.
        if(value.type() == QVariant::UInt &&
           param_template.type == QVariant::Int) {
            value = value.toInt();
        }
        else {
            CLogInfo log;
            log.setMsg(QString("The value specified for %1 has an incorrect type.")
                .arg(key));
            log.setSrc(CLogInfo::ESource::node);
            log.setName(m_name);
            log.setStatus(CLogInfo::EStatus::warning);
            log.printMessage();
            return false;
        }
    }

    param_template.value = value;

    return true;
}

void CNodeConfig::addFilename(QString key, QString name, QString description,
                              QString def_val)
{
    if(m_parameter_template_map.contains(key)) {
        qWarning() << "Overwriting the parameter"
                   << key << "in" << getName();
    }

    struct SParameterTemplate param_template;
    param_template.name = name;
    param_template.type = QVariant::String;
    param_template.value = def_val;
    param_template.description = description;

    m_parameter_template_map.insert(key, param_template);
}

void CNodeConfig::addBool(QString key, QString name, QString description,
                          bool def_val /* = false*/)
{
    if(m_parameter_template_map.contains(key)) {
        qWarning() << "Overwriting the parameter"
                   << key << "in" << getName();
    }

    struct SParameterTemplate param_template;
    param_template.name = name;
    param_template.type = QVariant::Bool;
    param_template.value = def_val;
    param_template.description = description;

    m_parameter_template_map.insert(key, param_template);
}

void CNodeConfig::addInt(QString key, QString name, QString description,
                         qint32 def_val/* = 0*/)
{
    if(m_parameter_template_map.contains(key)) {
            qWarning() << "Overwriting the parameter"
                       << key << "in" << getName();
    }

    struct SParameterTemplate param_template;
    param_template.name = name;
    param_template.type = QVariant::Int;
    param_template.value = def_val;
    param_template.description = description;

    m_parameter_template_map.insert(key, param_template);
}

void CNodeConfig::addUInt(QString key, QString name, QString description,
                          quint32 def_val/* = 0*/)
{
    if(m_parameter_template_map.contains(key)) {
            qWarning() << "Overwriting the parameter"
                       << key << "in" << getName();
    }

    struct SParameterTemplate param_template;
    param_template.name = name;
    param_template.type = QVariant::UInt;
    param_template.value = def_val;
    param_template.description = description;

    m_parameter_template_map.insert(key, param_template);
}

void CNodeConfig::addString(QString key, QString name, QString description,
                            QString def_val)
{
    if(m_parameter_template_map.contains(key)) {
            qWarning() << "Overwriting the parameter"
                       << key << "in" << getName();
    }

    struct SParameterTemplate param_template;
    param_template.name = name;
    param_template.type = QVariant::String;
    param_template.value = def_val;
    param_template.description = description;

    m_parameter_template_map.insert(key, param_template);
}

void CNodeConfig::addInput(QString name, QString msg_type)
{
    m_input_templates.append(SGateTemplate(name, msg_type));
}

void CNodeConfig::addOutput(QString name, QString msg_type)
{
    m_output_templates.append(SGateTemplate(name, msg_type));
}

const CNodeConfig::SParameterTemplate *CNodeConfig::getParameter(QString key) const
{
    if(!m_parameter_template_map.contains(key)) {
        qWarning() << "The parameter" << key << "has not been found.";
        return nullptr;
    }

    return &m_parameter_template_map[key];
}

QStringList CNodeConfig::getAllParameters() const
{
    return m_parameter_template_map.keys();
}

const QList<CNodeConfig::SGateTemplate> &CNodeConfig::getInputTemplates() const
{
    return m_input_templates;
}

const QList<CNodeConfig::SGateTemplate> &CNodeConfig::getOutputTemplates() const
{
    return m_output_templates;
}


//------------------------------------------------------------------------------
// Public Slots


//------------------------------------------------------------------------------
// Private Functions


//------------------------------------------------------------------------------
// Private Slots
