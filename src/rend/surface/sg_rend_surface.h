#ifndef SG_REND_SURFACE_H
#define SG_REND_SURFACE_H

#define VK_NO_PROTOTYPES
#include "volk.h"
#include "sg_base.h"

SgResult createWindowSurface(SgApp *pApp);
SgResult getSurfaceAttributes(SgApp *pApp);

#endif
