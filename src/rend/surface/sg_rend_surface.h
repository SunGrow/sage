#ifndef SG_REND_SURFACE_H
#define SG_REND_SURFACE_H

#define VK_NO_PROTOTYPES
#include "volk.h"
#include "sg_base.h"

SgResult createWindowSurface(const VkInstance* pInstance, SgWindow* pWindow, VkSurfaceKHR* pSurface);
SgResult getSurfaceAttributes(const VkPhysicalDevice* pPhysicalDevice, const VkSurfaceKHR* pSurface, SurfaceAttributes* pSurfaceAttributes);

#endif
