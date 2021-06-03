#ifndef SG_ASSETS_H
#define SG_ASSETS_H 1
#include "lz4.h"
#include "sg_base.h"
#include "sg_scene.h"

typedef struct SgProjects {
	char** ppProjectDirs;
	SgSize count;
} SgProjects;

SgResult sgLoadProjectDirs(const char* pPath, SgProjects* pProjects);

typedef struct SgProjectLayout {
	char* generalSettingsPath;
	char* keyContextsPath;
	char* scenesPath;
	char* dataPath;
	char* buildDir;

	cJSON* projectJson;
} SgProjectLayout;

SgResult sgLoadProjectLayout(const char*      pPath,
                             SgProjectLayout* pProjectLayout);

// TODO: request vertex format from json
typedef struct SgObjVertex {
	v3 vert;
	v3 norm;
	v2 tex;
} SgObjVertex;

typedef enum SgAssetCompression { SG_ASSET_COMPRESSION_LZ4 } SgAssetCompression;

typedef struct SgTextureAssets {
	char**       ppBytesArray;
	SgSize *     pWidthsArray, *pHeightsArray, *pChannelsArray, *pSizesArray;
	const char** ppNamesArray;
	SgSize       count;
	SgAssetCompression compression;
} SgTextureAssets;

typedef struct SgMeshAssets {
	const char** ppNamesArray;
	SgSize*      pVertexBufferSizes;
	SgSize*      pIndexBufferSizes;
	SgSize*      pCompressedSizesArray;
	u*           pVertexSizes;
	char**       ppBytesArray;

	SgSize             count;
	SgAssetCompression compression;
} SgMeshAssets;

typedef struct SgAssets {
	SgAssetVersion  version;
	SgTextureAssets textureAssets;
	SgMeshAssets    meshAssets;

	cJSON* pAssetsJson;
} SgAssetFiles;

SgResult sgFilesRead(const char*   pPath,
                     const char*   pTargetPath,
                     SgAssetFiles* pAssets);
SgResult sgFilesCompress(SgAssetFiles* pInAssets, SgAssetFiles* pOutAssets);
SgResult sgFilesWrite(SgAssetFiles* pAssets, const char* pOutPath);
SgResult sgFilesClear(SgAssetFiles* pAssets);

typedef struct SgProject {
	SgProjectLayout*  pProjectLayout;
	char*             pProjectDirPath;
	SgSceneAssets*    pSceneAssets;
	SgSceneResources* pSceneResources;
	SgAssetFiles      assets;
} SgProject;

SgResult sgProjectRead(char*            pProjectDirPath,
                       SgProjectLayout* pProjectLayout,
                       SgProject*       pProject);
// Constructs a compressed file data file, scene files and a project file
SgResult sgProjectWrite(SgProject* pProject);

#endif
