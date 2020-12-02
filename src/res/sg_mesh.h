#ifndef SG_MESH
#define SG_MESH

#include "sage_base.h"

// Mesh
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
//

// Mesh
void SgLoadMesh(const char *pPath, SgMesh **ppMesh);
void SgUnloadMesh(SgMesh **ppMesh);

#endif
