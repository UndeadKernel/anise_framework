#include "interface.h"
#include "ipfixparsernode.h"

extern "C"
{
void configure(CNodeConfig &config)
{
    CIpfixparserNode::configure(config);
}

CNode *maker(const CNodeConfig &config)
{
    return new CIpfixparserNode(config);
}
}

