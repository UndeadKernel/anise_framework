#include "interface.h"
#include "plotnode.h"

extern "C"
{
void configure(CNodeConfig &config)
{
    CPlotNode::configure(config);
}

CNode *maker(const CNodeConfig &config)
{
    return new CPlotNode(config);
}
}

