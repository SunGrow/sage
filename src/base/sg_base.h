#ifndef SAGE_BASE
#define SAGE_BASE
#define _DEBUG

#include <stddef.h>
#include <stdint.h>
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vk_mem_alloc.h"

#include <pthread.h>
#include <stdlib.h>

#include "../log/sg_log.h"
#include "cJSON.h"  // TODO: wrapper
#include "sg_types.h"

#define SG_DEFINE_HANDLE(object) typedef struct object##_T* object;
#define NUMOF(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BIT(n) (1UL << (n))
#define SG_CALLOC_NUM(object, num) \
	(object) = (calloc((num), sizeof((*object))));
#define SG_MALLOC_NUM(object, num) \
	(object) = (malloc((num) * sizeof((*object))));
#define SG_REALLOC_NUM(object, num, error)                     \
	void* tmpptr = realloc((object), (sizeof(*object)) * (num)); \
	if (tmpptr == NULL) {                                        \
		sgLogError(error);                                         \
	} else {                                                     \
		object = tmpptr;                                           \
	}
#define SG_STRETCHALLOC(object, num, error) \
	if ((object) == NULL) {                   \
		SG_MALLOC_NUM(object, num);             \
	} else {                                  \
		SG_REALLOC_NUM(object, num, error);     \
	}

typedef struct SurfaceAttributes {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkSurfaceFormatKHR       format;
	VkPresentModeKHR         presentMode;
} SurfaceAttributes;

enum {
	SG_THREADS_COUNT      = 64,
	SG_FRAME_QUEUE_LENGTH = 3,
};

typedef enum SgWindowCreateFlagBits {
	SG_APP_CURSOR_HIDDEN     = BIT(0),
	SG_APP_WINDOW_RESIZABLE  = BIT(1),
	SG_APP_WINDOW_FULLSCREEN = BIT(2),
} SgAppCreateFlagBits;
typedef SgFlags SgAppCreateFlags;

typedef struct SgWindowConfig {
	SgAppCreateFlags createFlags;
	SgSize           dimentions[2];
	const char*      pName;
} SgWindowConfig;

typedef struct SgGraphicsConfig {
	SgSize mssa;
} SgGraphicsConfig;

typedef struct SgAppConfig {
	SgWindowConfig   windowConfig;
	SgGraphicsConfig graphicsConfig;
	cJSON*           configJSON;
} SgAppConfig;

typedef GLFWwindow SgWindow;

typedef struct SgApp {
	SgWindow*  pWindow;
	VkInstance instance;
#ifdef _DEBUG
	VkDebugUtilsMessengerEXT debugCallback;
#endif
	VkSurfaceKHR     surface;
	VkPhysicalDevice physicalDevice;
	// TODO: Do it more generic.
	// With an array of queues and their priorities
	SgSize             graphicsQueueFamilyIdx;
	SurfaceAttributes  surfaceAttributes;
	VkDevice           device;
	VkSampleCountFlags msaaSampleCount;
	VkQueue            graphicsQueue;
	VmaAllocator       allocator;

	pthread_t threads[SG_THREADS_COUNT];
	// No secondary command buffer pools atm
	VkCommandPool pCommandPools[SG_FRAME_QUEUE_LENGTH * SG_THREADS_COUNT];
	SgSize        commandPoolCount;

	// Synchorization primitives
	VkSemaphore pFrameReadySemaphore[SG_FRAME_QUEUE_LENGTH];
	VkSemaphore pFrameFinishedSemaphore[SG_FRAME_QUEUE_LENGTH];
	SgSize      currentFrame;
	SgSize      frameImageIndex;
	VkFence     pFrameFences[SG_FRAME_QUEUE_LENGTH];
	VkFence     pFrameImageInFlightFences[SG_FRAME_QUEUE_LENGTH + 6];

	VkFormat lowDepthStencil;
	VkFormat highDepthStencil;

	VkRect2D*   pScissor;
	VkViewport* pViewport;
	// Config
	SgAppConfig appConfig;
} SgApp;

typedef enum SgResourceTypeFlagBits {
	SG_RESOURCE_TYPE_TEXTURE_2D = BIT(0),
	SG_RESOURCE_TYPE_MESH       = BIT(1),
	SG_RESOURCE_TYPE_UNIFORM    = BIT(2),
	SG_RESOURCE_TYPE_INDICES    = BIT(3),
	SG_RESOURCE_TYPE_STAGING    = BIT(4),  // Hidden from the outer API atm
} SgResourceTypeFlagBits;
typedef SgFlags                  SgResourceTypeFlags;
static const SgResourceTypeFlags SG_RESOURCE_TYPE_REQIRE_STAGING_MASK =
    SG_RESOURCE_TYPE_MESH | SG_RESOURCE_TYPE_INDICES;
static const SgResourceTypeFlags SG_RESOURCE_TYPE_IS_IMAGE_MASK =
    SG_RESOURCE_TYPE_TEXTURE_2D;

typedef struct SgBufferData {
	VkBuffer      buffer;
	void*         bytes;
	SgSize        size;
	VmaAllocation allocation;
} SgBufferData;

typedef struct SgImageData {
	VkImage       image;
	void*         bytes;
	VkExtent3D    extent;
	SgSize        size;
	VmaAllocation allocation;
	VkFormat      format;
} SgImageData;

typedef struct SgImageView {
	VkImageView imageView;
} SgImageView;
typedef enum SgShaderStageFlagBits {
	SG_SHADER_STAGE_VERTEX_BIT   = VK_SHADER_STAGE_VERTEX_BIT,
	SG_SHADER_STAGE_GEOMETRY_BIT = VK_SHADER_STAGE_GEOMETRY_BIT,
	SG_SHADER_STAGE_FRAGMENT_BIT = VK_SHADER_STAGE_FRAGMENT_BIT,
	SG_SHADER_STAGE_COMPUTE_BIT  = VK_SHADER_STAGE_COMPUTE_BIT,
} SgShaderStageFlagBits;
typedef SgFlags SgShaderStageFlags;

typedef struct SgShader {
	VkShaderModule     shader;
	SgShaderStageFlags stage;
} SgShader;

typedef struct SgShaderPass {
	VkPipeline       pipeline;
	VkPipelineLayout pipelineLayout;
} SgShaderPass;

typedef struct SgResourceBinding {
	SgResourceTypeFlags type;
	SgShaderStageFlags  stage;
	SgSize              binding;
	SgSize              setBinding;
} SgResourceBinding;

typedef struct SgSetBindings {
	SgResourceBinding* pBindings;
	SgSize             bindingCount;
} SgSetBindings;

typedef struct SgResource {
	SgBufferData        dataBuffer;
	SgImageData         image;
	VkImageView         imageView;
	VkSampler           imageSampler;
	SgBufferData        stagingBuffer;
	SgResourceTypeFlags type;

	SgSize* pCommandBufferID;

	void*  lastBytes;
	SgSize lastSize;

	const char* pName;
} SgResource;

typedef struct SgResourceMap {
	struct hashmap*  pResourceMap;
	VkCommandBuffer* pCommadBuffers;
	SgSize           commandBufferCount;

	VkCommandBuffer* pCommadBuffersForSpecificUpdate;

	VkFence       fence;
	VkCommandPool commandPool;
} SgResourceMap;

typedef struct SgSwapchain {
	VkSwapchainKHR swapchain;
	VkImage*       pFrameImages;
	VkImageView*   pFrameImageViews;
	SgImageData    depthImage;
	SgImageView    depthImageView;
	SgImageData    blendImage;
	SgImageView    blendImageView;
	VkFramebuffer* pFrameBuffers;
	VkExtent2D     extent;
	SgSize         imageCount;
} SgSwapchain;

typedef struct SgGraphicsPipelineBuilder {
	VkPipelineShaderStageCreateInfo* pShaderStages;
	SgSize                           shaderStageCount;

	VkPipelineVertexInputStateCreateInfo   vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkViewport                             viewport;
	VkRect2D                               scissor;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState    colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo   multisampling;
	VkPipelineLayout                       pipelineLayout;
	VkPipelineDepthStencilStateCreateInfo  depthStencil;
} SgGraphicsPipelineBuilder;

typedef struct SgComputePipelineBuilder {
	VkPipelineShaderStageCreateInfo shaderStage;
	VkPipelineLayout                pipelineLayout;
} SgComputePipelineBuilder;

typedef struct SgUpdateCommands {
	VkCommandBuffer* pCommandBuffers;
	SgResourceMap*   pResourceMap;
} SgUpdateCommands;

// Really does look like a file, doesn't it?
typedef struct SgData {
	void*  bytes;
	size_t size;
} SgData;

#ifdef __unix__
typedef struct SgFile {
	char*              pBytes;
	unsigned long long size;
} SgFile;
#else
#ifdef _WIN32
#include <windows.h>
typedef struct SgFile {
	HANDLE        hFile;
	LPVOID        pBytes;
	LARGE_INTEGER sizeLI;
	LONGLONG      size;
} SgFile;
#endif
#endif

SgResult sgOpenFile(const char* path, SgFile** ppFile);
SgResult sgWriteFile(const char* path, SgFile* pFile);
SgResult sgCloseFile(SgFile** ppFile);

#endif
