#define FAST_OBJ_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "sg_mesh.h"
#include "stdlib.h"
#include "stdio.h"
#include "sage_base.h"
#include "fast_obj.h"
#include "meshoptimizer.h"

typedef struct SgMeshItem {
	const char*     pPath;
	uint32_t        offsetid;
} SgMeshItem;

static int keyCompare(const void* a, const void* b, void* udata) {
	const SgMeshItem* keyA = a;
	const SgMeshItem* keyB = b;
	return strcmp(keyA->pPath, keyB->pPath);
}

static uint64_t keyHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SgMeshItem* key = item;
    return hashmap_sip(key->pPath, strlen(key->pPath), seed0, seed1);
}


static bool keyIter(const void *item, void *udata) {
	const SgMeshItem* key = item;
	sgLogInfo("[MeshIter]: Path: %s, Offset: %d", key->pPath, key->offsetid);
    return true;
}

SgResult sgCreateMeshSet(SgMeshSet** ppMeshArray) {
	SgMeshSet* pMeshArray = *ppMeshArray;
	SG_CALLOC_NUM(pMeshArray, 1);
	pMeshArray->meshMap = hashmap_new(sizeof(SgMeshItem), 1, 0, 0, keyHash, keyCompare, NULL);
	*ppMeshArray=pMeshArray;
	return SG_SUCCESS;
}

uint32_t sgAddMesh(const char* pPath, SgMeshSet** ppMeshArray) {
	SgMeshSet* pMeshArray = *ppMeshArray;
	SgMeshItem meshItem;
	meshItem.pPath = pPath;
	SgMeshItem* pMeshItem = hashmap_get(pMeshArray->meshMap, &meshItem);
	if (pMeshItem) {
		*ppMeshArray = pMeshArray;
		return pMeshItem->offsetid;
	}
	meshItem.offsetid = sgLoadMesh(pPath, &pMeshArray);
	hashmap_set(pMeshArray->meshMap, &meshItem);
	*ppMeshArray = pMeshArray;
	return meshItem.offsetid;
}

uint32_t* sgGetMeshID(const char* pPath, const SgMeshSet* pMeshArray) {
	SgMeshItem meshItem;
	meshItem.pPath = pPath;
	SgMeshItem* pMeshItem = hashmap_get(pMeshArray->meshMap, &meshItem);
	// Could get away with it 'cause hashmap is allocated
	return &pMeshItem->offsetid;
}

// Returns indices count
static uint32_t loadOBJ(const char* pPath, SgVertex** ppVertices) {
	fastObjMesh *pObj = fast_obj_read(pPath);
	uint32_t totalIndices = 0;
	for (uint32_t i = 0; i < pObj->face_count; ++i) {
		totalIndices += 3 * (pObj->face_vertices[i] - 2);
	}
	SgVertex* pVertices;
	SG_MALLOC_NUM(pVertices, totalIndices);
	uint32_t vertexOffset = 0;
	uint32_t indexOffset = 0;
	for (uint32_t i = 0; i < pObj->face_count; ++i) {
		for (uint32_t j = 0; j < pObj->face_vertices[i]; ++j) {
			fastObjIndex gi = pObj->indices[indexOffset + j];

			SgVertex v = {
				.vert = {
					pObj->positions[gi.p * 3 + 0],
					pObj->positions[gi.p * 3 + 1],
					pObj->positions[gi.p * 3 + 2],
				},
				.norm = {
					pObj->normals[gi.n * 3 + 0],
					pObj->normals[gi.n * 3 + 1],
					pObj->normals[gi.n * 3 + 2],
				},
			    .tex = {
					pObj->texcoords[gi.t * 2 + 0],
					1.0f - pObj->texcoords[gi.t * 2 + 1],
				}, 
			};

			// triangulation
			if (j >= 3) {
				pVertices[vertexOffset + 0] = pVertices[vertexOffset - 3];
				pVertices[vertexOffset + 1] = pVertices[vertexOffset - 1];
			}
			pVertices[vertexOffset] = v;
			++vertexOffset;
		}
		indexOffset += pObj->face_vertices[i];
	}
	fast_obj_destroy(pObj);
	*ppVertices = pVertices;
	return totalIndices;
}

uint32_t sgLoadMesh(const char *pPath, SgMeshSet **ppMesh) {
	// obj mesh load

	SgMeshSet* pMesh = *ppMesh;

	// TODO: Fix zis constant realloc
	SG_STRETCHALLOC(pMesh->pVertexOffsets, pMesh->meshCount+1, "[ALLOC]");
	SG_STRETCHALLOC(pMesh->pIndexOffsets, pMesh->meshCount+1, "[ALLOC]");
	SG_STRETCHALLOC(pMesh->pIndexSizes, pMesh->meshCount+1, "[ALLOC]");
	SG_STRETCHALLOC(pMesh->pVertexSizes, pMesh->meshCount+1, "[ALLOC]");
	SgVertex *pVertices;
	uint32_t totalIndices = loadOBJ(pPath, &pVertices);
	// MeshLoad
	//
	// DEBUG
	if (1) {
		uint32_t totalVertices = totalIndices;
		SG_STRETCHALLOC(pMesh->pVertices, pMesh->vertexCount+totalIndices, "[ALLOC]");
		SG_STRETCHALLOC(pMesh->pIndices,  pMesh->indexCount+totalVertices, "[ALLOC]");
		memcpy(&pMesh->pVertices[pMesh->vertexCount], pVertices, totalVertices*sizeof(SgVertex));
		for (uint32_t i = 0; i < totalIndices; ++i) {
			pMesh->pIndices[pMesh->indexCount+i] = i;
		}
		pMesh->pIndexSizes[pMesh->meshCount]    = totalIndices;
		pMesh->pVertexSizes[pMesh->meshCount]   = totalVertices;
		pMesh->pIndexOffsets[pMesh->meshCount]  = pMesh->indexCount;
		pMesh->pVertexOffsets[pMesh->meshCount] = pMesh->vertexCount;
		pMesh->indexCount  += totalIndices;
		pMesh->vertexCount += totalVertices;
		pMesh->meshCount += 1;
		*ppMesh = pMesh;
		free(pVertices);
		return pMesh->meshCount-1;
	}
	uint32_t *premap;
	SG_MALLOC_NUM(premap, totalIndices);

	uint32_t totalVertices = meshopt_generateVertexRemap(premap, NULL, totalIndices, pVertices, totalIndices, sizeof(*pMesh->pVertices));

	// Return value fillup
	SG_STRETCHALLOC(pMesh->pIndices, pMesh->indexCount+totalIndices, "[ALLOC]");
	SG_STRETCHALLOC(pMesh->pVertices, pMesh->vertexCount+totalVertices, "[ALLOC]");

	meshopt_remapVertexBuffer(&pMesh->pVertices[pMesh->vertexCount], pVertices, totalIndices, sizeof(*pMesh->pVertices), premap);
	meshopt_remapIndexBuffer(&pMesh->pIndices[pMesh->indexCount], NULL, totalIndices, premap);
	meshopt_optimizeVertexCache(&pMesh->pIndices[pMesh->indexCount], &pMesh->pIndices[pMesh->indexCount], totalIndices, totalVertices);
	meshopt_optimizeVertexFetch(&pMesh->pVertices[pMesh->vertexCount], &pMesh->pIndices[pMesh->indexCount], totalIndices, &pMesh->pVertices[pMesh->vertexCount], totalVertices, sizeof(*pMesh->pVertices));

	pMesh->pIndexOffsets[pMesh->meshCount]  = pMesh->indexCount;
	pMesh->pVertexOffsets[pMesh->meshCount] = pMesh->vertexCount;
	pMesh->indexCount  += totalIndices;
	pMesh->vertexCount += totalVertices;
	pMesh->pIndexSizes[pMesh->meshCount]  = totalIndices;
	pMesh->pVertexSizes[pMesh->meshCount] = totalVertices;
	pMesh->meshCount += 1;
	*ppMesh = pMesh;
	free(pVertices);
	sgLogInfo("[Res]: Mesh Read Successfull");

	free(premap);
	return pMesh->meshCount-1;
}

void sgUnloadMesh(SgMesh **ppMesh) {
	(*ppMesh)->indexCount = 0;
	(*ppMesh)->vertexCount = 0;
	free((*ppMesh)->pIndices);
	free((*ppMesh)->pVertices);
	free(*ppMesh);
}


void sgTransformMesh(const SgMeshTransformInfo *pTransformInfo, uint32_t offset, uint32_t vertCount, SgVertex *pVertices) {
	for (uint32_t i = 0; i < vertCount; ++i) {
		pVertices[offset+i].vert[0] *= pTransformInfo->scale[0];
		pVertices[offset+i].vert[1] *= pTransformInfo->scale[1];
		pVertices[offset+i].vert[2] *= pTransformInfo->scale[2];

		pVertices[offset+i].vert[0] += pTransformInfo->move[0];
		pVertices[offset+i].vert[1] += pTransformInfo->move[1];
		pVertices[offset+i].vert[2] += pTransformInfo->move[2];
	}
}

void sgLoadTexture(const char *pPath, SgTexture **ppTexture) {
	SgTexture *pTexture = calloc(1, sizeof(pTexture[0]));
	pTexture->pixels = stbi_load(pPath, &pTexture->width, &pTexture->height,
	                            &pTexture->channels, STBI_rgb_alpha);
	if (!pTexture->pixels) {
		fprintf(stderr, "[Warning]: Texture on path < %s > not found\n", pPath);
	}
	pTexture->size = pTexture->height;
	pTexture->size *= pTexture->width;
	pTexture->size <<= 2;

	*ppTexture = pTexture;
}

void sgUnloadTexture(SgTexture **ppTexture) {
	stbi_image_free(ppTexture[0]->pixels);
	ppTexture[0]->pixels = NULL;
	ppTexture[0]->channels = 0;
	ppTexture[0]->width = 0;
	ppTexture[0]->height = 0;
	free(*ppTexture);
}



