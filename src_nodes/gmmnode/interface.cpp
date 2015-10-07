#include "interface.h"
#include "gmmnode.h"

extern "C"
{
void configure(CNodeConfig &config)
{
    CGmmNode::configure(config);
}

CNode *maker(const CNodeConfig &config)
{
    return new CGmmNode(config);
}
}

