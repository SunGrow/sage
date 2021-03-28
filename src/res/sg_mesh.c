#define FAST_OBJ_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "sg_mesh.h"
#include "stdlib.h"
#include "stdio.h"
#include "log.h"
#include "fast_obj.h"
#include "meshoptimizer.h"

uint32_t sgAddMesh(const char* pPath, SgMeshArray** ppMeshArray) {
	return 0;
}

uint32_t sgLoadMesh(const char *pPath, SgMesh **ppMesh) {
	// obj mesh load
	fastObjMesh *pObj = fast_obj_read(pPath);

	SgMesh* pMesh = calloc(1, sizeof(pMesh[0]));
	uint32_t totalindices = 0;
	for (uint32_t i = 0; i < pObj->face_count; ++i) {
		totalindices += 3 * (pObj->face_vertices[i] - 2);
	}
	SgVertex *pvertices = malloc(totalindices * sizeof(SgVertex));
	uint32_t vertexoffset = 0;
	uint32_t indexoffset = 0;
	for (uint32_t i = 0; i < pObj->face_count; ++i) {
		for (uint32_t j = 0; j < pObj->face_vertices[i]; ++j) {
			fastObjIndex gi = pObj->indices[indexoffset + j];

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
				pvertices[vertexoffset + 0] = pvertices[vertexoffset - 3];
				pvertices[vertexoffset + 1] = pvertices[vertexoffset - 1];
			}
			pvertices[vertexoffset] = v;
			++vertexoffset;
		}
		indexoffset += pObj->face_vertices[i];
	}
	fast_obj_destroy(pObj);
	// Debug
	if (0) {
		pMesh->pVertices =
		    realloc(pMesh->pVertices, totalindices * sizeof(SgVertex));
		pMesh->pIndices =
		    realloc(pMesh->pIndices, totalindices * sizeof(uint32_t));
		for (uint32_t i = 0; i < totalindices; ++i) {
			pMesh->pIndices[i]  = i;
			pMesh->pVertices[i] = pvertices[i];
		}
		pMesh->vertexCount = totalindices;
		pMesh->indexCount = totalindices;
		*ppMesh = pMesh;
		log_info("[Res]: Mesh Read Successfull");
		return 0;
	}
	uint32_t *premap = malloc(totalindices * sizeof(uint32_t));

	uint32_t totalvertices = meshopt_generateVertexRemap(
	    premap, NULL, totalindices, pvertices, totalindices, sizeof(SgVertex));

	// Return value fillup
	pMesh->pVertices = malloc(totalvertices * sizeof(SgVertex));
	pMesh->pIndices = malloc(totalindices * sizeof(uint32_t));

	meshopt_remapVertexBuffer(pMesh->pVertices, pvertices, totalindices,
	                          sizeof(SgVertex), premap);

	meshopt_remapIndexBuffer(pMesh->pIndices, NULL, totalindices, premap);
	pMesh->indexCount = totalindices;
	pMesh->vertexCount = totalvertices;
	*ppMesh = pMesh;
	free(pvertices);
	log_info("[Res]: Mesh Read Successfull");

	free(premap);
	return 0;
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



