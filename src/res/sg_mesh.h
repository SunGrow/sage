#ifndef SG_MESH
#define SG_MESH

#include "sage_base.h"
#include "sg_math.h"
#include "stb_image.h"
#include "hashmap.h"

// Mesh
typedef struct SgVertex {
	v3 vert;
	v3 norm;
	v2 tex;
} SgVertex;

typedef struct SgMesh {
	SgVertex   *pVertices;
	uint32_t   vertexCount;
	uint32_t   *pIndices;
	uint32_t   indexCount;
} SgMesh;
//

typedef struct SgMeshArray {
	SgVertex*          pVertices;
	uint32_t           vertexCount;
	uint32_t*          pVertexOffsets;
	uint32_t*          pIndices;
	uint32_t           indexCount;
	uint32_t*          pIndexOffsets;
	struct hashmap*    meshMap;      // Return an offset if is in. Add new and return new offset if is new
} SgMeshArray;

// Returns mesh offset to added mesh
uint32_t sgAddMesh(const char* pPath, SgMeshArray** ppMeshArray);
uint32_t sgLoadMesh(const char *pPath, SgMesh **ppMesh);
void     sgUnloadMesh(SgMesh **ppMesh);

typedef struct SgMeshTransformInfo {
	v3 move;
	v3 scale;
} SgMeshTransformInfo;

void sgTransformMesh(const SgMeshTransformInfo *pTransformInfo, uint32_t offset, uint32_t vertCount, SgVertex *pVertices);

typedef struct SgTexture {
	int32_t width, height, channels;
	uint32_t size;
	stbi_uc *pixels;
} SgTexture;

void sgLoadTexture(const char *pPath, SgTexture **ppTexture);
void sgUnloadTexture(SgTexture **ppTexture);

#endif
