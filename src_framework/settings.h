#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <QMap>
#include <QString>
#include <QVariant>


// Static class that stores the settings of the framework set
// ... by the user.
class CSettings
{
  public:
    static void set(QString setting, QVariant value);
    static QVariant get(QString setting);
    // Helper functions
    static bool machine();
    static bool progress();
    static bool log();

  private:
    static QMap<QString, QVariant> settings;

    // Class is not meant to be constructed;
    CSettings() {}
};

#endif // CSETTINGS_H
