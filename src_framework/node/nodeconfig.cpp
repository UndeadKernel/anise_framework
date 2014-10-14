#include "nodeconfig.h"
#include <QDebug>

//------------------------------------------------------------------------------
// Constructor and Destructor

CNodeConfig::CNodeConfig()
    : m_name("Node")
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

bool CNodeConfig::setParameter(QString key, QVariant value) const
{
    // Key exists?
    if(!m_parameter_template_map.contains(key)) {
        qWarning() << "CNodeConfig::setParameter():"
                   << "The parameter" << key << "has not been defined"
                   << "in the configuration template.";
        return false;
    }

    SParameterTemplate &param_template = m_parameter_template_map[key];

    // Verify that the supplied value matches the required value.
    if(value.type() != param_template.type) {
        qWarning() << "CNodeConfig::setParameter():"
                   << "The value specified for" << key
                   << "has an incorrect type.";
        return false;
    }

    param_template.value = value;

    return true;
}

void CNodeConfig::addFilename(QString key, QString name, QString description)
{
    if(m_parameter_template_map.contains(key)) {
        qWarning() << "CNodeConfig::addFilename(): Overwriting the parameter"
                   << key << "in" << getName();
    }

    struct SParameterTemplate param_template;
    param_template.name = name;
    param_template.type = QVariant::String;
    param_template.description = description;

    m_parameter_template_map.insert(key, param_template);
}

void CNodeConfig::addBool(QString key, QString name, QString description)
{
    if(m_parameter_template_map.contains(key)) {
        qWarning() << "CNodeConfig::addBool(): Overwriting the parameter"
                   << key << "in" << getName();
    }

    struct SParameterTemplate param_template;
    param_template.name = name;
    param_template.type = QVariant::Bool;
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
        qWarning() << "CNodeConfig::getParameter():"
                 << "The parameter" << key << "has not been found.";
        return nullptr;
    }

    return &m_parameter_template_map[key];
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


