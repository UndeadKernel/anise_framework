#include "interface.h"
#include "csvdumpnode.h"

extern "C"
{
void configure(CNodeConfig &config)
{
    CCsvdumpNode::configure(config);
}

CNode *maker(const CNodeConfig &config)
{
    return new CCsvdumpNode(config);
}
}

