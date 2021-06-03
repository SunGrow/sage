#ifndef SG_SCENE_H
#define SG_SCENE_H

#include "sg_base.h"
#include "sg_math.h"
#include "sg_types.h"
#define VK_NO_PROTOTYPES
#include "vk_mem_alloc.h"
#include "volk.h"

typedef struct SgSceneNode SgSceneNode;

typedef enum SgType {
	SG_TYPE_INVALID,
	SG_TYPE_SCALAR,
	SG_TYPE_VEC2,
	SG_TYPE_VEC3,
	SG_TYPE_VEC4,
	SG_TYPE_MAT2,
	SG_TYPE_MAT3,
	SG_TYPE_MAT4,
} SgType;

typedef enum SgComponentType {
	SG_COMPONENT_TYPE_INVALID,
	SG_COMPONENT_TYPE_R_8,   /* BYTE */
	SG_COMPONENT_TYPE_R_8U,  /* UNSIGNED_BYTE */
	SG_COMPONENT_TYPE_R_16,  /* SHORT */
	SG_COMPONENT_TYPE_R_16U, /* UNSIGNED_SHORT */
	SG_COMPONENT_TYPE_R_32U, /* UNSIGNED_INT */
	SG_COMPONENT_TYPE_R_32F, /* FLOAT */
} SgComponentType;

typedef enum SgBufferViewType {
	SG_BUFFER_VIEW_TYPE_INVALID,
	SG_BUFFER_VIEW_TYPE_INDICES,
	SG_BUFFER_VIEW_TYPE_VERTICES,
} SgBufferViewType;

typedef struct SgBuffer {
	SgSize loadSize;
	char*  pUri;
	void*  data;

	SgBufferData* pBufferData;
} SgBuffer;

typedef struct SgBufferView {
	SgBufferData*    pBuffer;
	SgSize           offset;
	SgSize           size;
	SgSize           stride;
	SgBufferViewType type;
} SgBufferView;

typedef struct SgImage {
	char*         pUri;
	SgBufferView* pBuferView;
	char*         pMimeType;

	SgImageData* pImageData;

	char* pName;
} SgImage;

typedef struct SgSampler {
	SgInt magFilter;
	SgInt minFilter;
	SgInt wrapS;
	SgInt wrapT;
} SgSampler;

typedef struct SgTextureAttr {
	SgImageData* pImage;
	SgSampler*   pSampler;

	char* pName;
} SgTextureAttr;

typedef struct SgTextureView {
	SgUInt             levelCount;
	SgUInt             layerCount;
	VkImageViewType    imageViewType;
	VkImageAspectFlags imageAspectFlags;
	char*              pTextureName;

	char* pName;
} SgTextureView;

typedef struct SgAccessor {
	SgComponentType componentType;
	SgBool          normalized;
	SgType          type;
	SgSize          offset;
	SgSize          count;
	SgSize          stride;
	SgBufferView*   pBufferView;
} SgAccessor;

typedef struct SgSkin {
	SgSceneNode** ppJoints;
	SgSize        jointCount;
	SgSceneNode*  pSkeleton;
	SgAccessor*   pInverseBindMatrices;

	char* pName;
} SgSkin;

typedef enum SgPrimitiveType {
	SG_PRIMITIVE_TYPE_POINTS,
	SG_PRIMITIVE_TYPE_LINES,
	SG_PRIMITIVE_TYPE_LINE_LOOP,
	SG_PRIMITIVE_TYPE_LINE_STRIP,
	SG_PRIMITIVE_TYPE_TRIANGLES,
	SG_PRIMITIVE_TYPE_TRIANGLE_STRIP,
	SG_PRIMITIVE_TYPE_TRIANGLE_FAN,
} SgPrimitiveType;

typedef enum SgAttributeType {
	SG_ATTRIBUTE_TYPE_INVALID,
	SG_ATTRIBUTE_TYPE_POSITION,
	SG_ATTRIBUTE_TYPE_NORMAL,
	SG_ATTRIBUTE_TYPE_TANGENT,
	SG_ATTRIBUTE_TYPE_TEXCOORD,
	SG_ATTRIBUTE_TYPE_COLOR,
	SG_ATTRIBUTE_TYPE_JOINTS,
	SG_ATTRIBUTE_TYPE_WEIGHTS,
} SgAttributeType;

typedef struct SgAttribute {
	char*           pName;
	SgAttributeType type;
	SgInt           index;
	SgAccessor*     data;
} SgAttribute;

typedef struct SgDracoMeshCompression {
	SgBufferView* pBufferView;
	SgAttribute*  attributes;
	SgSize        attributesCount;
} SgDracoMeshCompression;

typedef struct SgMeshoptMeshCompression {
	SgBufferView* pBufferView;
	SgAttribute*  attributes;
	SgSize        attributesCount;
} SgMeshoptMeshCompression;

typedef struct SgMorphTarget {
	SgAttribute* pAttributes;
	SgSize       attributesCount;
} SgMorphTarget;

typedef struct SgPBRMetalicRoughness {
	SgTextureView baseColorTexture;
	SgTextureView metallicRoughnessTexture;

	v4      baseColorFactor;
	SgFloat metallicFactor;
	SgFloat roughnessFactor;
} SgPBRMetalicRoughness;

typedef struct SgPBRSpecularGlossiness {
	SgTextureView diffuseTexture;
	SgTextureView specularGlossinessTexture;

	v4      diffuseFactor;
	v3      specularFactor;
	SgFloat glossinessFactor;
} SgPBRSpecularGlossiness;

typedef struct SgClearcoat {
	SgTextureView clearcoatTexture;
	SgTextureView clearcoatRoughnessTexture;
	SgTextureView clearcoatNormalTexture;

	SgFloat clearcoatFactor;
	SgFloat clearcoatRoughnessFactor;
} SgClearcoat;

typedef struct SgTransmission {
	SgTextureView transmissionTexture;
	SgFloat       transmissionFactor;
} SgTransmission;

typedef struct SgIOR {
	SgFloat ior;
} SgIOR;

typedef struct SgSpecular {
	SgTextureView specularTexture;
	v3            specularColorFactor;
	SgFloat       specularFactor;
} SgSpecular;

typedef struct SgSheen {
	SgTextureView sheenColorTexture;
	v3            sheenColorFactor;
	SgTextureView sheenRoughnessTexture;
	SgFloat       sheenRoughnessFactor;
} SgSheen;

typedef enum SgAlphaMode {
	SG_ALPHA_MODE_OPAQUE,
	SG_ALPHA_MODE_MASK,
	SG_ALPHA_MODE_BLEND,
} SgAlphaMode;

typedef struct SgShaderAttr {
	VkShaderModuleCreateInfo*        pShader;
	VkShaderStageFlags               stage;
	VkDescriptorSetLayoutCreateInfo* pSetLayout;

	char* pName;
} SgShaderAttr;

typedef struct SgMaterialAttr {
	SgBool                  hasPbrMetallicRoughness;
	SgBool                  hasPbrSpecularGlossiness;
	SgBool                  hasClearcoat;
	SgBool                  hasTransmission;
	SgBool                  hasIor;
	SgBool                  hasSpecular;
	SgBool                  hasSheen;
	SgPBRMetalicRoughness   pbrMetallicRoughness;
	SgPBRSpecularGlossiness pbrSpecularGlossiness;
	SgClearcoat             clearcoat;
	SgIOR                   ior;
	SgSpecular              specular;
	SgSheen                 sheen;
	SgTransmission          transmission;
	SgTextureView           normalTexture;
	SgTextureView           occlusionTexture;
	SgTextureView           emissiveTexture;
	v3                      emissiveFactor;
	SgAlphaMode             alphaMode;
	SgFloat                 alphaCutoff;
	SgBool                  doubleSided;
	SgBool                  unlit;

	SgShaderAttr** ppShaders;
	SgSize         shaderCount;

	char* pName;
} SgMaterialAttr;

typedef struct SgPrimitive {
	SgPrimitiveType          type;
	SgAccessor*              pIndices;
	SgMaterialAttr*          pMaterial;
	SgAttribute*             pAttributes;
	SgSize                   attributeCount;
	SgMorphTarget*           pTargets;
	SgSize                   targets_count;
	SgBool                   hasDracoMeshCompression;
	SgDracoMeshCompression   dracoMeshCompression;
	SgBool                   hasMeshoptMeshCompression;
	SgMeshoptMeshCompression meshoptMeshCompression;
} SgPrimitive;

typedef struct SgMesh {
	SgPrimitive* pPrimitives;
	SgSize       primitivesCount;
	SgFloat*     pWeights;
	SgSize       weightsCount;
	char**       ppTargetNames;
	SgSize       targetNamesCount;
	char*        pName;
} SgMesh;

typedef struct SgCameraPerspective {
	SgFloat aspectRatio;
	SgFloat yFov;
	SgFloat zFar;
	SgFloat zNear;
} SgCameraPerspective;

typedef struct SgCameraOrthographic {
	SgFloat xMag;
	SgFloat yMag;
	SgFloat zFar;
	SgFloat zNear;
} SgCameraOrthographic;

typedef enum SgCameraType {
	SG_CAMERA_TYPE_INVALID,
	SG_CAMERA_TYPE_PERSPECTIVE,
	SG_CAMERA_TYPE_ORTHOGRAPHIC,
} SgCameraType;

typedef struct SgCamera {
	SgCameraType type;
	union {
		SgCameraPerspective  perspective;
		SgCameraOrthographic orthographic;
	} data;

	char* pName;
} SgCamera;

typedef enum SgLightType {
	SG_LIGHT_TYPE_INVALID,
	SG_LIGHT_TYPE_DIRECTIONAL,
	SG_LIGHT_TYPE_POINT,
	SG_LIGHT_TYPE_SPOT,
} SgLightType;

typedef struct SgLight {
	v3          color;
	SgFloat     intensity;
	SgLightType type;
	SgFloat     range;
	SgFloat     spotInnerConeAngle;
	SgFloat     spotOuterConeAngle;

	char* pName;
} SgLight;

struct SgSceneNode {
	SgSceneNode*  pParentNode;
	SgSceneNode** pChildrenNodes;
	SgSize        childrenCount;
	SgSkin*       pSkin;
	SgMesh*       pMesh;
	SgCamera*     pCamera;
	SgLight*      pLight;
	SgFloat*      pWeights;
	SgSize        weightsCount;
	SgBool        hasTranslation;
	SgBool        hasScale;
	SgBool        hasMatrix;
	v3            translation;
	v4            rotation;
	v3            scale;
	m4            matrix;

	char* pName;
};

typedef struct SgPassAttachment {
	VkFormat            format;
	SgSize*             pSampleCount;
	VkAttachmentLoadOp  loadOp;
	VkAttachmentStoreOp storeOp;
	VkAttachmentLoadOp  stencilLoadOp;
	VkAttachmentStoreOp stencilStoreOp;
	VkImageLayout       initialLayout;
	VkImageLayout       finalLayout;

	char* pName;
} SgPassAttachment;

typedef enum SgPassTypeFlagBits {
	SG_PASS_TYPE_RENDER  = BIT(0),
	SG_PASS_TYPE_COMPUTE = BIT(1),
} SgPassTypeFlagBits;
typedef SgPassTypeFlagBits SgPassTypeFlags;

typedef struct SgPass {
	SgPassTypeFlags       type;
	SgPassAttachment*     pPassAttachments;
	SgSize                passAttachmentsCount;
	VkSubpassDescription* pSubpassDescriptions;
	SgSize                subpassDescriptionsCount;
	VkSubpassDependency*  pDependencies;
	SgSize                dependenciesCount;

	char* pName;
} SgPass;

typedef struct SgFramebuffer {
	SgPass*      pRenderPass;
	SgSize       width;
	SgSize       height;
	VkImageView* pImageViewAttachments;
	SgSize       layers;

	char* pName;
} SgFramebuffer;

typedef struct SgScene {
	SgSceneNode** ppNodes;
	SgSize        nodeCount;

	SgPass** ppPasses;
	SgSize   passCount;

	char* pName;
} SgScene;

typedef enum SgAssetVersion { SG_ASSET_VERION_0_1 } SgAssetVersion;

typedef struct SgSceneAssets {
	SgScene*       pScenes;
	SgSize         sceneCount;
	SgAssetVersion version;
} SgSceneAssets;

typedef struct SgSceneResources {
	SgPass* pPasses;
	SgSize  passCount;

	SgFramebuffer* pFramebuffers;
	SgSize         framebufferCount;

	SgMaterialAttr* pTemplateMaterialAttributes;
	SgSize          materialAttributeCount;

	SgShaderAttr* pShaderAttributes;
	SgSize        shaderAttributeCount;

	VkShaderModuleCreateInfo* pShaderModules;
	char**                    ppShaderModuleNames;
	SgSize                    shaderModuleCount;

	VkDescriptorSetLayoutCreateInfo* pSetLayouts;
	char**                           ppSetLayoutNames;
	SgSize                           setLayoutCount;
} SgSceneResources;

typedef struct SgAssetLoadInfo {
	SgFile* pAssetFile;
} SgSceneAssetLoadInfo;

SgResult sgLoadSceneAssets(const SgSceneAssetLoadInfo* assetLoadInfo,
                           SgSceneAssets*              pAssets);

#endif
