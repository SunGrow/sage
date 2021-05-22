#ifndef SG_MESH
#define SG_MESH

#include "hashmap.h"
#include "sg_base.h"
#include "sg_math.h"
#include "stb_image.h"

// Mesh
typedef struct SgVertex {
	v3 vert;
	v3 norm;
	v2 tex;
} SgVertex;

typedef struct SgMesh {
	SgVertex* pVertices;
	SgSize vertexCount;
	SgSize* pIndices;
	SgSize indexCount;
} SgMesh;
//

typedef struct SgMeshSet {
	SgVertex* pVertices;
	SgSize vertexCount;
	SgSize* pVertexOffsets;
	SgSize* pIndexSizes;
	SgSize* pVertexSizes;
	SgSize* pIndices;
	SgSize indexCount;
	SgSize* pIndexOffsets;
	SgSize meshCount;
	struct hashmap* meshMap;  // Return an offset id

	const char* indicesResourceName;
} SgMeshSet;

// Returns mesh offset to added mesh
SgResult sgCreateMeshSet(SgMeshSet** ppMeshArray);
SgSize sgAddMesh(const char* pPath, SgMeshSet** ppMeshArray);
SgSize* sgGetMeshID(const char* pPath, const SgMeshSet* pMeshArray);
SgSize sgLoadMesh(const char* pPath, SgMeshSet** ppMesh);
void sgUnloadMesh(SgMesh** ppMesh);

///

typedef struct SgTexture {
	int32_t width, height, channels;
	SgSize size;
	stbi_uc* pixels;
} SgTexture;

void sgLoadTexture(const char* pPath, SgTexture** ppTexture);
void sgUnloadTexture(SgTexture** ppTexture);

#endif
