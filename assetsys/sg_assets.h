#ifndef SG_ASSETS_H
#define SG_ASSETS_H 1
#include "lz4.h"
#include "sg_base.h"
#include "sg_math.h"

// TODO: request vertex format from json
typedef struct SgObjVertex {
	v3 vert;
	v3 norm;
	v2 tex;
} SgObjVertex;

typedef enum SgAssetVersion { SG_ASSET_VERION_0_1 } SgAssetVersion;

typedef enum SgAssetCompression { SG_ASSET_COMPRESSION_LZ4 } SgAssetCompression;

typedef struct SgTextureAssets {
	char** ppBytesArray;
	unsigned long long *pWidthsArray, *pHeightsArray, *pChannelsArray,
	    *pSizesArray;
	const char** ppNamesArray;
	unsigned long long count;
	SgAssetCompression compression;
} SgTextureAssets;

typedef struct SgMeshAssets {
	const char** ppNamesArray;
	unsigned long long* pVertexBufferSizes;
	unsigned long long* pIndexBufferSizes;
	unsigned long long* pCompressedSizesArray;
	unsigned* pVertexSizes;
	char** ppBytesArray;

	unsigned long long count;
	SgAssetCompression compression;
} SgMeshAssets;

typedef struct SgAssets {
	SgAssetVersion version;
	SgTextureAssets textureAssets;
	SgMeshAssets meshAssets;

	cJSON* pAssetsJson;
} SgAssets;

SgResult sgAssetsRead(const char* pPath,
                      const char* pTargetPath,
                      SgAssets* pAssets);
SgResult sgAssetsCompress(SgAssets* pInAssets, SgAssets* pOutAssets);
SgResult sgAssetsWrite(SgAssets* pAssets, const char* pOutPath);
SgResult sgAssetsClear(SgAssets* pAssets);
#endif
