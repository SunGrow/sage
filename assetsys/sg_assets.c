#include "sg_assets.h"
#include <string.h>
#include "cJSON.h"
#include "sg_log.h"

#define FAST_OBJ_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "fast_obj.h"
#include "meshoptimizer.h"
#include "stb_image.h"

static SgResult sgFillProjectsDirs(cJSON* projectsJson, SgProjects* pProjects) {
	cJSON* projectsArr = cJSON_GetObjectItem(projectsJson, "projectDirs");
	if (projectsArr == NULL) {
		sgLogWarn("Project array is not provided");
		return -1;
	}
	pProjects->count = cJSON_GetArraySize(projectsArr);
	SG_CALLOC_NUM(pProjects->ppProjectDirs, pProjects->count);
	for (SgSize i = 0; i < pProjects->count; ++i) {
		cJSON* projectsItem         = cJSON_GetArrayItem(projectsArr, i);
		pProjects->ppProjectDirs[i] = cJSON_GetStringValue(projectsItem);
		if (pProjects->ppProjectDirs[i] == NULL) {
			sgLogWarn("No project path string provided");
		}
	}

	return SG_SUCCESS;
}

SgResult sgLoadProjectDirs(const char* pPath, SgProjects* pProjects) {
	SgFile* pFile;
	sgOpenFile(pPath, &pFile);
	const char* pError;
	cJSON*      projectsJson = cJSON_ParseWithOpts(pFile->pBytes, &pError, 1);
	if (projectsJson == NULL) {
		if (pError != NULL) {
			sgLogError("[JSON]: Error before: %s\n", pError);
			return -1;
		}
	}
	sgFillProjectsDirs(projectsJson, pProjects);
	return SG_SUCCESS;
}
static SgResult sgFillProjectLayout(cJSON*           projectJson,
                                    SgProjectLayout* pProjectLayout) {
	cJSON* generalSettingsPath =
	    cJSON_GetObjectItem(projectJson, "generalSettingsPath");
	if (generalSettingsPath == NULL) {
		sgLogWarn("General settings path not found");
		return -1;
	}
	pProjectLayout->generalSettingsPath =
	    cJSON_GetStringValue(generalSettingsPath);
	if (pProjectLayout->generalSettingsPath == NULL) {
		sgLogWarn("No general settings path string provided");
	}

	cJSON* keyContextsPath = cJSON_GetObjectItem(projectJson, "keyContextsPath");
	if (keyContextsPath == NULL) {
		sgLogWarn("Key contexts path not found");
		return -1;
	}
	pProjectLayout->keyContextsPath = cJSON_GetStringValue(keyContextsPath);
	if (pProjectLayout->keyContextsPath == NULL) {
		sgLogWarn("No key contexts path string provided");
	}

	cJSON* scenesPath = cJSON_GetObjectItem(projectJson, "scenesPath");
	if (scenesPath == NULL) {
		sgLogWarn("Scenes path not found");
		return -1;
	}
	pProjectLayout->scenesPath = cJSON_GetStringValue(scenesPath);
	if (pProjectLayout->scenesPath == NULL) {
		sgLogWarn("Scenes path string provided");
	}

	cJSON* dataPath = cJSON_GetObjectItem(projectJson, "dataPath");
	if (dataPath == NULL) {
		sgLogWarn("Data path not found");
		return -1;
	}
	pProjectLayout->dataPath = cJSON_GetStringValue(dataPath);
	if (pProjectLayout->dataPath == NULL) {
		sgLogWarn("Data path string provided");
	}

	cJSON* buildDir = cJSON_GetObjectItem(projectJson, "buildDir");
	if (buildDir == NULL) {
		sgLogWarn("Build path not found");
		return -1;
	}
	pProjectLayout->buildDir = cJSON_GetStringValue(buildDir);
	if (pProjectLayout->buildDir == NULL) {
		sgLogWarn("Build path string provided");
	}

	return SG_SUCCESS;
}

SgResult sgLoadProjectLayout(const char*      pPath,
                             SgProjectLayout* pProjectLayout) {
	char pFullPath[32768];
	snprintf(pFullPath, sizeof(pFullPath), "%s/%s", pPath, "project.json");
	SgFile* pFile;
	sgOpenFile(pFullPath, &pFile);
	const char* pError;
	cJSON*      projectJson = cJSON_ParseWithOpts(pFile->pBytes, &pError, 1);
	if (projectJson == NULL) {
		if (pError != NULL) {
			sgLogError("[JSON]: Error before: %s\n", pError);
			return -1;
		}
	}
	sgFillProjectLayout(projectJson, pProjectLayout);
	pProjectLayout->projectJson = projectJson;

	return SG_SUCCESS;
}

const char* getExt(const char* pPath) {
	const char* pExt = strrchr(pPath, '.');
	if (pExt == NULL || pExt == pPath) {
		return "";
	}
	return pExt + 1;
}

static SgResult sgReadJpg(const char* pPath,
                          char**      ppBytes,
                          SgSize*     pWidth,
                          SgSize*     pHeight,
                          SgSize*     pChannels,
                          SgSize*     pSize) {
	*ppBytes = stbi_load(pPath, pWidth, pHeight, pChannels, STBI_rgb_alpha);
	if (!*ppBytes) {
		fprintf(stderr, "[Warning]: Texture on path < %s > not found\n", pPath);
	}
	*pSize = *pHeight;
	*pSize *= *pWidth;
	*pSize <<= 2;
	return SG_SUCCESS;
}

static SgResult sgReadTexture(const char* pPath,
                              char**      ppBytes,
                              SgSize*     pWidth,
                              SgSize*     pHeight,
                              SgSize*     pChannels,
                              SgSize*     pSize) {
	const char* pExt = getExt(pPath);
	if (strcmp(pExt, "jpg") == 0 || strcmp(pExt, "jpeg") == 0
	    || strcmp(pExt, "jpe") == 0 || strcmp(pExt, "jfif") == 0) {
		sgReadJpg(pPath, ppBytes, pWidth, pHeight, pChannels, pSize);
	}

	return SG_SUCCESS;
}

static SgSize sgLoadOBJ(const char* pPath, SgObjVertex** ppVertices) {
	fastObjMesh* pObj         = fast_obj_read(pPath);
	SgSize       totalIndices = 0;
	for (SgSize i = 0; i < pObj->face_count; ++i) {
		totalIndices += 3 * (pObj->face_vertices[i] - 2);
	}
	SgObjVertex* pVertices;
	SG_CALLOC_NUM(pVertices, totalIndices + 1);
	SgSize vertexOffset = 0;
	SgSize indexOffset  = 0;
	for (SgSize i = 0; i < pObj->face_count; ++i) {
		for (SgSize j = 0; j < pObj->face_vertices[i]; ++j) {
			fastObjIndex gi = pObj->indices[indexOffset + j];

			SgObjVertex v = {
			    .vert =
			        {
			            pObj->positions[gi.p * 3 + 0],
			            pObj->positions[gi.p * 3 + 1],
			            pObj->positions[gi.p * 3 + 2],
			        },
			    .norm =
			        {
			            pObj->normals[gi.n * 3 + 0],
			            pObj->normals[gi.n * 3 + 1],
			            pObj->normals[gi.n * 3 + 2],
			        },
			    .tex =
			        {
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

static SgResult sgReadObj(const char* pPath,
                          char**      ppBytes,
                          SgSize*     pVertexBufferSize,
                          SgSize*     pIndexBufferSize,
                          unsigned*   pVertexSize) {
	SgObjVertex* pVertices;
	SgSize       totalIndices = sgLoadOBJ(pPath, &pVertices);
	SgSize*      premap;
	SG_MALLOC_NUM(premap, totalIndices);

	SgSize totalVertices = meshopt_generateVertexRemap(
	    premap, NULL, totalIndices, pVertices, totalIndices, sizeof(*pVertices));

	// Return value fillup
	SgSize*      pIndices;
	SgObjVertex* pTargetVertices;
	SG_CALLOC_NUM(pIndices, totalIndices);
	SG_CALLOC_NUM(pTargetVertices, totalVertices);

	meshopt_remapVertexBuffer(pTargetVertices, pVertices, totalIndices,
	                          sizeof(*pTargetVertices), premap);
	meshopt_remapIndexBuffer(pIndices, NULL, totalIndices, premap);
	meshopt_optimizeVertexCache(pIndices, pIndices, totalIndices, totalVertices);
	meshopt_optimizeVertexFetch(pTargetVertices, pIndices, totalIndices,
	                            pTargetVertices, totalVertices,
	                            sizeof(*pTargetVertices));

	*pVertexSize       = sizeof(*pVertices);
	*pVertexBufferSize = sizeof(*pTargetVertices) * totalVertices;
	*pIndexBufferSize  = sizeof(*pIndices) * totalIndices;
	SG_CALLOC_NUM(*ppBytes, *pVertexBufferSize + *pIndexBufferSize);
	memcpy(*ppBytes, pTargetVertices, *pVertexBufferSize);
	memcpy(*ppBytes + *pVertexBufferSize, pIndices, *pIndexBufferSize);

	return SG_SUCCESS;
}

static SgResult sgReadMesh(const char* pPath,
                           char**      ppBytes,
                           SgSize*     pVertexBufferSize,
                           SgSize*     pIndexBufferSize,
                           unsigned*   pVertexSize) {
	const char* pExt = getExt(pPath);
	if (strcmp(pExt, "obj") == 0) {
		sgReadObj(pPath, ppBytes, pVertexBufferSize, pIndexBufferSize, pVertexSize);
	}

	return SG_SUCCESS;
}

static SgResult sgAssetsMeshessRead(const char*   pTargetPath,
                                    cJSON*        meshArray,
                                    SgAssetFiles* pAssets) {
	SgSize meshArraySize = cJSON_GetArraySize(meshArray);
	char   pFullPath[32768];
	pAssets->meshAssets.count = meshArraySize;
	SG_CALLOC_NUM(pAssets->meshAssets.ppNamesArray, meshArraySize);
	SG_CALLOC_NUM(pAssets->meshAssets.pVertexBufferSizes, meshArraySize);
	SG_CALLOC_NUM(pAssets->meshAssets.pIndexBufferSizes, meshArraySize);
	SG_CALLOC_NUM(pAssets->meshAssets.pVertexSizes, meshArraySize);
	SG_CALLOC_NUM(pAssets->meshAssets.ppBytesArray, meshArraySize);
	// TODO: Use Job system to load (once it is implemented)
	for (SgSize i = 0; i < meshArraySize; ++i) {
		cJSON* mesh = cJSON_GetArrayItem(meshArray, i);
		if (mesh == NULL) {
			sgLogError(
			    "Something clearly went wrong. Mesh array item is not present "
			    "while the json array that contains it is of the size of %d",
			    meshArraySize);
			return -1;
		}
		cJSON* meshName = cJSON_GetObjectItem(mesh, "name");
		if (meshName == NULL) {
			sgLogWarn(
			    "Oops, you probably forgot to inlcude a mesh name under the index "
			    "of %d! Silly you!",
			    i);
			continue;
		}
		cJSON* meshPath = cJSON_GetObjectItem(mesh, "path");
		if (meshPath == NULL) {
			sgLogWarn(
			    "Oops, you probably forgot to inlcude a mesh path under the index "
			    "of %d! Silly you!",
			    i);
			continue;
		}
		// TODO: use reference to a string object
		pAssets->meshAssets.ppNamesArray[i] = cJSON_GetStringValue(meshName);
		if (pAssets->meshAssets.ppNamesArray[i] == NULL) {
			sgLogWarn(
			    "Oops, your mesh name under the index of %d is not a string! You "
			    "silly jokester!",
			    i);
			continue;
		}
		const char* pPath = cJSON_GetStringValue(meshPath);
		if (pPath == NULL) {
			sgLogWarn(
			    "Oops, your mesh path under the index of %d is not a string! You "
			    "silly jokester!",
			    i);
			continue;
		}
		snprintf(pFullPath, sizeof(pFullPath), "%s/%s", pTargetPath, pPath);
		sgReadMesh(pFullPath, &pAssets->meshAssets.ppBytesArray[i],
		           &pAssets->meshAssets.pVertexBufferSizes[i],
		           &pAssets->meshAssets.pIndexBufferSizes[i],
		           &pAssets->meshAssets.pVertexSizes[i]);
	}

	return SG_SUCCESS;
}

static SgResult sgAssetsTexturesRead(const char*   pTargetPath,
                                     cJSON*        textureArray,
                                     SgAssetFiles* pAssets) {
	SgSize textureArraySize = cJSON_GetArraySize(textureArray);
	char   pFullPath[32768];
	pAssets->textureAssets.count = textureArraySize;
	SG_CALLOC_NUM(pAssets->textureAssets.ppNamesArray, textureArraySize);
	SG_CALLOC_NUM(pAssets->textureAssets.pSizesArray, textureArraySize);
	SG_CALLOC_NUM(pAssets->textureAssets.ppBytesArray, textureArraySize);
	SG_CALLOC_NUM(pAssets->textureAssets.pWidthsArray, textureArraySize);
	SG_CALLOC_NUM(pAssets->textureAssets.pHeightsArray, textureArraySize);
	SG_CALLOC_NUM(pAssets->textureAssets.pChannelsArray, textureArraySize);
	// TODO: Use Job system to load (once it is implemented)
	for (SgSize i = 0; i < textureArraySize; ++i) {
		cJSON* texture = cJSON_GetArrayItem(textureArray, i);
		if (texture == NULL) {
			sgLogError(
			    "Something clearly went wrong. Texture array item is not present "
			    "while the json array that contains it is of the size of %d",
			    textureArraySize);
			return -1;
		}
		cJSON* textureName = cJSON_GetObjectItem(texture, "name");
		if (textureName == NULL) {
			sgLogWarn(
			    "Oops, you probably forgot to inlcude a texture name under the index "
			    "of %d! Silly you!",
			    i);
			continue;
		}
		cJSON* texturePath = cJSON_GetObjectItem(texture, "path");
		if (texturePath == NULL) {
			sgLogWarn(
			    "Oops, you probably forgot to inlcude a texture path under the index "
			    "of %d! Silly you!",
			    i);
			continue;
		}
		// TODO: use reference to a string object
		pAssets->textureAssets.ppNamesArray[i] = cJSON_GetStringValue(textureName);
		if (pAssets->textureAssets.ppNamesArray[i] == NULL) {
			sgLogWarn(
			    "Oops, your texture name under the index of %d is not a string! You "
			    "silly jokester!",
			    i);
			continue;
		}
		const char* pPath = cJSON_GetStringValue(texturePath);
		if (pPath == NULL) {
			sgLogWarn(
			    "Oops, your texture path under the index of %d is not a string! You "
			    "silly jokester!",
			    i);
			continue;
		}
		snprintf(pFullPath, sizeof(pFullPath), "%s/%s", pTargetPath, pPath);
		sgReadTexture(pFullPath, &pAssets->textureAssets.ppBytesArray[i],
		              &pAssets->textureAssets.pWidthsArray[i],
		              &pAssets->textureAssets.pHeightsArray[i],
		              &pAssets->textureAssets.pChannelsArray[i],
		              &pAssets->textureAssets.pSizesArray[i]);
	}

	return SG_SUCCESS;
}

SgResult sgFilesRead(const char*   pPath,
                     const char*   pTargetPath,
                     SgAssetFiles* pAssets) {
	SgFile* pFile;
	sgOpenFile(pPath, &pFile);
	const char* pError;
	cJSON*      assetsJson = cJSON_ParseWithOpts(pFile->pBytes, &pError, 1);
	if (assetsJson == NULL) {
		if (pError != NULL) {
			sgLogError("[JSON]: Error before: %s\n", pError);
			return -1;
		}
	}
	cJSON* textureArray = cJSON_GetObjectItem(assetsJson, "textures");
	cJSON* meshArray    = cJSON_GetObjectItem(assetsJson, "meshes");
	sgAssetsTexturesRead(pTargetPath, textureArray, pAssets);
	sgAssetsMeshessRead(pTargetPath, meshArray, pAssets);
	pAssets->pAssetsJson = assetsJson;

	sgCloseFile(&pFile);
	return SG_SUCCESS;
}

static SgResult sgTexturesCompress(SgAssetFiles* pInAssets,
                                   SgAssetFiles* pOutAssets) {
	pOutAssets->textureAssets.count = pInAssets->textureAssets.count;
	SG_CALLOC_NUM(pOutAssets->textureAssets.ppNamesArray,
	              pInAssets->textureAssets.count);
	SG_CALLOC_NUM(pOutAssets->textureAssets.pSizesArray,
	              pInAssets->textureAssets.count);
	SG_CALLOC_NUM(pOutAssets->textureAssets.ppBytesArray,
	              pInAssets->textureAssets.count);
	SG_CALLOC_NUM(pOutAssets->textureAssets.pWidthsArray,
	              pInAssets->textureAssets.count);
	SG_CALLOC_NUM(pOutAssets->textureAssets.pHeightsArray,
	              pInAssets->textureAssets.count);
	SG_CALLOC_NUM(pOutAssets->textureAssets.pChannelsArray,
	              pInAssets->textureAssets.count);
	// TODO: Paralellize
	for (SgSize i = 0; i < pInAssets->textureAssets.count; ++i) {
		pOutAssets->textureAssets.pChannelsArray[i] =
		    pInAssets->textureAssets.pChannelsArray[i];
		pOutAssets->textureAssets.pHeightsArray[i] =
		    pInAssets->textureAssets.pHeightsArray[i];
		pOutAssets->textureAssets.pWidthsArray[i] =
		    pInAssets->textureAssets.pWidthsArray[i];
		pOutAssets->textureAssets.pSizesArray[i] =
		    pInAssets->textureAssets.pSizesArray[i];
		pOutAssets->textureAssets.ppNamesArray[i] =
		    pInAssets->textureAssets.ppNamesArray[i];

		uint64_t compressBound =
		    LZ4_compressBound(pInAssets->textureAssets.pSizesArray[i]);
		SG_CALLOC_NUM(pOutAssets->textureAssets.ppBytesArray[i], compressBound);

		SgSize compressedSize = LZ4_compress_default(
		    pInAssets->textureAssets.ppBytesArray[i],
		    pOutAssets->textureAssets.ppBytesArray[i],
		    pInAssets->textureAssets.pSizesArray[i], compressBound);
		SG_REALLOC_NUM(pOutAssets->textureAssets.ppBytesArray[i], compressedSize,
		               "Texture reallocation error");
		pOutAssets->textureAssets.pSizesArray[i] = compressedSize;
	}

	return SG_SUCCESS;
}

static SgResult sgMeshCompress(SgAssetFiles* pInAssets,
                               SgAssetFiles* pOutAssets) {
	pOutAssets->meshAssets.count = pInAssets->meshAssets.count;
	SG_CALLOC_NUM(pOutAssets->meshAssets.ppNamesArray,
	              pOutAssets->meshAssets.count);
	SG_CALLOC_NUM(pOutAssets->meshAssets.pVertexBufferSizes,
	              pOutAssets->meshAssets.count);
	SG_CALLOC_NUM(pOutAssets->meshAssets.pIndexBufferSizes,
	              pOutAssets->meshAssets.count);
	SG_CALLOC_NUM(pOutAssets->meshAssets.pVertexSizes,
	              pOutAssets->meshAssets.count);
	SG_CALLOC_NUM(pOutAssets->meshAssets.ppBytesArray,
	              pOutAssets->meshAssets.count);
	SG_CALLOC_NUM(pOutAssets->meshAssets.pCompressedSizesArray,
	              pOutAssets->meshAssets.count);
	// TODO: Paralellize
	for (SgSize i = 0; i < pInAssets->textureAssets.count; ++i) {
		pOutAssets->meshAssets.pIndexBufferSizes[i] =
		    pInAssets->meshAssets.pIndexBufferSizes[i];
		pOutAssets->meshAssets.pVertexBufferSizes[i] =
		    pInAssets->meshAssets.pVertexBufferSizes[i];
		pOutAssets->meshAssets.pVertexSizes[i] =
		    pInAssets->meshAssets.pVertexSizes[i];
		pOutAssets->meshAssets.ppNamesArray[i] =
		    pInAssets->meshAssets.ppNamesArray[i];

		uint64_t compressBound =
		    LZ4_compressBound(pInAssets->meshAssets.pVertexBufferSizes[i]
		                      + pInAssets->meshAssets.pIndexBufferSizes[i]);
		SG_CALLOC_NUM(pOutAssets->meshAssets.ppBytesArray[i], compressBound);

		SgSize compressedSize = LZ4_compress_default(
		    pInAssets->meshAssets.ppBytesArray[i],
		    pOutAssets->meshAssets.ppBytesArray[i],
		    pInAssets->meshAssets.pVertexBufferSizes[i]
		        + pInAssets->meshAssets
		              .pIndexBufferSizes[i],  // Will be used as an original size
		                                      // for fast decompress
		    compressBound);
		SG_REALLOC_NUM(pOutAssets->meshAssets.ppBytesArray[i], compressedSize,
		               "Mesh reallocation error");
		pOutAssets->meshAssets.pCompressedSizesArray[i] = compressedSize;
	}

	return SG_SUCCESS;
}

SgResult sgFilesCompress(SgAssetFiles* pInAssets, SgAssetFiles* pOutAssets) {
	sgMeshCompress(pInAssets, pOutAssets);
	sgTexturesCompress(pInAssets, pOutAssets);
	return SG_SUCCESS;
}

SgResult sgFilesWrite(SgAssetFiles* pAssets, const char* pOutPath) {
	char   pFullPath[32768];
	cJSON* assets = cJSON_CreateObject();
	cJSON *textureArray, *meshArray;
	textureArray = cJSON_CreateArray();
	meshArray    = cJSON_CreateArray();
	cJSON_AddItemToObject(assets, "textures", textureArray);
	cJSON_AddItemToObject(assets, "meshes", meshArray);
	for (SgSize i = 0; i < pAssets->textureAssets.count; ++i) {
		cJSON* texture = cJSON_CreateObject();
		cJSON_AddStringToObject(texture, "name",
		                        pAssets->textureAssets.ppNamesArray[i]);
		cJSON_AddByteStringToObject(texture, "bytes",
		                            pAssets->textureAssets.ppBytesArray[i],
		                            pAssets->textureAssets.pSizesArray[i]);
		cJSON_AddNumberToObject(texture, "width",
		                        pAssets->textureAssets.pWidthsArray[i]);
		cJSON_AddNumberToObject(texture, "height",
		                        pAssets->textureAssets.pHeightsArray[i]);
		cJSON_AddNumberToObject(texture, "channels",
		                        pAssets->textureAssets.pChannelsArray[i]);
		cJSON_AddNumberToObject(texture, "size",
		                        pAssets->textureAssets.pSizesArray[i]);
		cJSON_AddNumberToObject(texture, "compression",
		                        pAssets->textureAssets.compression);
		cJSON_AddItemToArray(textureArray, texture);
	}
	for (SgSize i = 0; i < pAssets->meshAssets.count; ++i) {
		cJSON* mesh = cJSON_CreateObject();
		cJSON_AddStringToObject(mesh, "name", pAssets->meshAssets.ppNamesArray[i]);
		cJSON_AddByteStringToObject(mesh, "bytes",
		                            pAssets->meshAssets.ppBytesArray[i],
		                            pAssets->meshAssets.pCompressedSizesArray[i]);
		cJSON_AddNumberToObject(mesh, "vertexSize",
		                        pAssets->meshAssets.pVertexSizes[i]);
		cJSON_AddNumberToObject(mesh, "vertexBufferSize",
		                        pAssets->meshAssets.pVertexBufferSizes[i]);
		cJSON_AddNumberToObject(mesh, "indexBufferSize",
		                        pAssets->meshAssets.pIndexBufferSizes[i]);
		cJSON_AddNumberToObject(mesh, "compression",
		                        pAssets->meshAssets.compression);
		cJSON_AddItemToArray(meshArray, mesh);
	}
	SgFile file;
	snprintf(pFullPath, sizeof(pFullPath), "%s/%s", pOutPath, "binAssets.json");
	file.pBytes = cJSON_PrintUnformattedWithLengthGet(assets, &file.size);
	sgWriteFile(pFullPath, &file);
	return SG_SUCCESS;
}

SgResult sgFilesClear(SgAssetFiles* pAssets) {
	return SG_SUCCESS;
}

SgResult sgFillSceneResources(cJSON* sceneResources, SgProject* pProject) {
	return SG_SUCCESS;
}

static SgResult sgReadSceneResources(char* pScenesPath, SgProject* pProject) {
	SgFile* pFile;
	sgOpenFile(pScenesPath, &pFile);
	const char* pError;
	cJSON*      sceneResourcesJson = cJSON_ParseWithOpts(pFile->pBytes, &pError, 1);
	if (sceneResourcesJson == NULL) {
		if (pError != NULL) {
			sgLogError("[JSON]: Error before: %s\n", pError);
			return -1;
		}
	}
	sgFillSceneResources(sceneResourcesJson, pProject);

	return SG_SUCCESS;
}

static SgResult sgReadSceneAssets(char* pScenesPath, SgProject* pProject) {
	sgFilesRead(pScenesPath, pProject->pProjectDirPath, &pProject->assets);
	SgAssetFiles outAssets;
	sgFilesCompress(&pProject->assets, &outAssets);
	sgFilesClear(&pProject->assets);
	pProject->assets = outAssets;
	return SG_SUCCESS;
}

SgResult sgProjectRead(char*            pProjectDirPath,
                       SgProjectLayout* pProjectLayout,
                       SgProject*       pProject) {
	pProject->pProjectLayout  = pProjectLayout;
	pProject->pProjectDirPath = pProjectDirPath;
	char pScenesPath[32768];
	snprintf(pScenesPath, sizeof(pScenesPath), "%s/%s", pProject->pProjectDirPath, pProject->pProjectLayout->scenesPath);
	sgReadSceneResources(pScenesPath, pProject);
	sgReadSceneAssets(pScenesPath, pProject);
	
	return SG_SUCCESS;
}

// Constructs a compressed file data file, scene files and a project file
SgResult sgProjectWrite(SgProject* pProject) {
	char pDataPath[32768];
	snprintf(pDataPath, sizeof(pDataPath), "%s/%s/%s", pProject->pProjectLayout->buildDir,
	         pProject->pProjectDirPath, pProject->pProjectLayout->dataPath);
	sgFilesWrite(&pProject->assets, pDataPath);
	return SG_SUCCESS;
}
