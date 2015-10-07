#include "interface.h"
#include "entropyfeaturesnode.h"

extern "C"
{
void configure(CNodeConfig &config)
{
    CEntropyfeaturesNode::configure(config);
}

CNode *maker(const CNodeConfig &config)
{
    return new CEntropyfeaturesNode(config);
}
}

