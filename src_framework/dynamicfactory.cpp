#include "dynamicfactory.h"
#include "loginfo.h"
#include <QDir>
#include <QStringList>
#include <QDebug>
#include <QCoreApplication>

//------------------------------------------------------------------------------
// Constructor and Destructor

CDynamicFactory::CDynamicFactory()
{

}


//------------------------------------------------------------------------------
// Public Functions

void CDynamicFactory::loadLibraries(QString folder, QString filter, int flags)
{
    QDir exec_dir(QCoreApplication::applicationDirPath());
    // Look in the given dictionary.
    QDir dir = exec_dir.filePath(folder);
    // Search only for files.
    dir.setFilter(QDir::Files);
    // Only list files ending with .so
    QStringList filterlist;
    filterlist << filter;
    dir.setNameFilters(filterlist);

    QStringList list = dir.entryList();

    for(auto it = list.begin(); it != list.end(); ++it) {
        QString filename = dir.filePath(*it);
        // Open the library file.
        void *handle = dlopen(filename.toLocal8Bit().constData(), flags);
        if(handle == NULL) {
            CLogInfo log;
            log.setSrc(CLogInfo::ESource::framework);
            log.setStatus(CLogInfo::EStatus::error);
            log.setMsg(QString("There was an error loading the library '")
                + (*it) + "'." + dlerror());
            log.printMessage();
            continue;
        }

        // Let the base class choose what to do with the library.
        addLibrary(handle, filename);
    }
}


//------------------------------------------------------------------------------
// Private Functions
