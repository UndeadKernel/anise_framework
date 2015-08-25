#include "nodestarttask.h"
#include "../progressinfo.h"
#include "../loginfo.h"
#include "../settings.h"
#include <QDebug>
#define tadasa

//------------------------------------------------------------------------------
// Constructor and Destructor

CNodeStartTask::CNodeStartTask(CNode &node, QObject *parent /* = 0 */)
    : QObject(parent)
    , m_node(node)
{

}


//------------------------------------------------------------------------------
// Public Functions

void CNodeStartTask::run()
{
    bool success;
    // Information about the starting progress of the node.
    CProgressInfo progress;
    CLogInfo log;
    progress.setSrc(CProgressInfo::ESource::node);
    progress.setName(m_node.getConfig().getName());
    progress.setState(CProgressInfo::EState::init);

#ifdef tadasa
    log.setSrc(CLogInfo::ESource::node);
    log.setName(m_node.getConfig().getName());
#endif
    // Should we log the process?
    if(CSettings::progress()) {
        progress.setMsg(CProgressInfo::EMsg::start);
        progress.printProgress();
    }

    // Start initializing the node.
    success = m_node.start();

    // Shall we print that we are finished initializing?
    if(success && CSettings::progress()) {
        progress.setMsg(CProgressInfo::EMsg::stop);
        progress.printProgress();
    }

    // Report if we did not succeed.
    if(!success) {
        // Report the failure
        progress.setMsg(CProgressInfo::EMsg::error);
        progress.setInfo("Initialization failed.");
        progress.printProgress();
      #ifdef tadasa
        log.setMsg("Initialization failed");
        log.setStatus(CLogInfo::EStatus::error);
        log.printMessage();
    #endif
    }

    // Tell whoever is interested that the task has been finished (either
    // ... successfull or not).
    emit taskFinished(success);
}

//------------------------------------------------------------------------------
// Private Functions
