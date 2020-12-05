#ifndef SAGE_RES_H_
#define SAGE_RES_H_ 1

#include "sage_core.h"
#include "sage_math.h"

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

void sgLoadMesh(const char *pPath, SgMesh **ppMesh);
void sgUnloadMesh(SgMesh **ppMesh);

typedef struct SgMeshTransformInfo {
	v3 move;
	v3 scale;
} SgMeshTransformInfo;

void sgTransformMesh(const SgMeshTransformInfo *pTransformInfo, uint32_t vertCount, SgVertex *pVertices);

#endif
