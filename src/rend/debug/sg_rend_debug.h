#ifndef SG_REND_DEBUG_H
#define SG_REND_DEBUG_H

#define VK_NO_PROTOTYPES
#include "volk.h"
#include "sg_base.h"

VkDebugReportCallbackEXT registerDebugCallback(SgApp *pApp);

#endif
