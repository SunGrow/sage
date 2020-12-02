#ifndef SAGE_RES_H_
#define SAGE_RES_H_ 1

#include "sage_core.h"

/* Make mesh load fit with the engine theme */
typedef struct SgVertex {
	float vx, vy, vz;
	float nx, ny, nz;
	float tu, tv;
} SgVertex;

typedef struct SgMesh {
	SgVertex   *pVertices;
	uint32_t   vertexCount;
	uint32_t   *pIndices;
	uint32_t   indexCount;
} SgMesh;

void SgLoadMesh(const char *pPath, SgMesh **ppMesh);
void SgUnloadMesh(SgMesh **ppMesh);


#endif
