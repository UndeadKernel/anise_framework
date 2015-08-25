#include "settings.h"

QMap<QString, QVariant> CSettings::settings;


void CSettings::set(QString setting, QVariant value)
{
    settings[setting] = value;
}

QVariant CSettings::get(QString setting)
{
    return settings[setting];
}

bool CSettings::machine()
{
    return CSettings::get("machine").toBool();
}

bool CSettings::progress()
{
    return CSettings::get("progress").toBool();
}

bool CSettings::log()
{
    return CSettings::get("log").toBool();
}
