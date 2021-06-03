#ifndef SG_REND_ALLOCATOR_H
#define SG_REND_ALLOCATOR_H

#include "sg_base.h"

SgResult createAllocator(const VkPhysicalDevice* pPhysicalDevice,
                         const VkDevice*         pDevice,
                         const VkInstance*       pInstance,
                         VmaAllocator*           pAllocator);

#endif
