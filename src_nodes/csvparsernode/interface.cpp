#include "interface.h"
#include "csvparsernode.h"

extern "C"
{
void configure(CNodeConfig &config)
{
    CCsvparserNode::configure(config);
}

CNode *maker(const CNodeConfig &config)
{
    return new CCsvparserNode(config);
}
}

