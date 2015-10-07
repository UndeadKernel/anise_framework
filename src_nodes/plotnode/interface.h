#ifndef INTERFACE_H
#define INTERFACE_H

#include "node/nodeconfig.h"

class CNode;

extern "C"
{
void configure(CNodeConfig &config);
CNode *maker(const CNodeConfig &config);
}

#endif // INTERFACE_H

