#ifndef SG_MESH
#define SG_MESH

#include "sg_base.h"
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

typedef struct SgMeshSet {
	SgVertex*          pVertices;
	uint32_t           vertexCount;
	uint32_t*          pVertexOffsets;
	uint32_t*          pIndexSizes;
	uint32_t*          pVertexSizes;
	uint32_t*          pIndices;
	uint32_t           indexCount;
	uint32_t*          pIndexOffsets;
	uint32_t           meshCount;
	struct hashmap*    meshMap;      // Return an offset id

	const char*        indicesResourceName;
} SgMeshSet;


// Returns mesh offset to added mesh
SgResult  sgCreateMeshSet(SgMeshSet** ppMeshArray);
uint32_t  sgAddMesh(const char* pPath, SgMeshSet** ppMeshArray);
uint32_t* sgGetMeshID(const char* pPath, const SgMeshSet* pMeshArray);
uint32_t  sgLoadMesh(const char *pPath, SgMeshSet **ppMesh);
void      sgUnloadMesh(SgMesh **ppMesh);


///

typedef struct SgTexture {
	int32_t width, height, channels;
	uint32_t size;
	stbi_uc *pixels;
} SgTexture;

void sgLoadTexture(const char *pPath, SgTexture **ppTexture);
void sgUnloadTexture(SgTexture **ppTexture);

#endif
