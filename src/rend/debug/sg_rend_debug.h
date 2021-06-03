#ifndef SG_REND_DEBUG_H
#define SG_REND_DEBUG_H

#define VK_NO_PROTOTYPES
#include "sg_base.h"
#include "volk.h"

SgResult registerDebugCallback(const VkInstance*         pInstance,
                               VkDebugUtilsMessengerEXT* debugMessenger);

#endif
