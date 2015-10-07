#include "interface.h"
#include "csvdumpdata.h"

extern "C"
{
CData *maker()
{
    return new CCsvdumpData();
}
}

