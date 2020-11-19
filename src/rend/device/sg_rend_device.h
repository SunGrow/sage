#ifndef SG_REND_DEVICE_H
#define SG_REND_DEVICE_H

#define VK_NO_PROTOTYPES
#include "volk.h"
#include "sage_base.h"

SgResult getPhysicalDevice(SgApp *pApp);
SgResult getLogicalDevice(SgApp *pApp);

#endif
