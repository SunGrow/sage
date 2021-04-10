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
// TODO: Thread wrapper to easily change out apis when needed
#include <pthread.h>
#include "../log/sage_log.h"

#define SG_DEFINE_HANDLE( object ) typedef struct object##_T *object;
#define NUMOF(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BIT(n)     (1UL << (n))
#define SG_CALLOC_NUM(object, num) (object) = (calloc((num), sizeof((*object))));
#define SG_MALLOC_NUM(object, num) (object) = (malloc((num) * sizeof((*object))));
#define SG_REALLOC_NUM(object, num, error)\
	void* tmpptr = realloc((object), (sizeof(*object)) * (num));\
	if (tmpptr == NULL) {\
		sgLogError(error);\
	} else {\
		object = tmpptr;\
	}
#define SG_STRETCHALLOC(object, num, error)\
	if ((object) == NULL) {\
		SG_MALLOC_NUM(object, num);\
	} else {\
		SG_REALLOC_NUM(object, num, error);\
	}\

typedef enum {
	SG_SUCCESS = 0,
} SgResult;
enum {
	SG_FALSE = 0,
	SG_TRUE  = 1,
};
typedef uint32_t SgBool;
typedef uint32_t SgFlags;


typedef struct SurfaceAttributes {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR presentMode;
} SurfaceAttributes;

enum {
	SG_THREADS_COUNT = 64,
	SG_FRAME_QUEUE_LENGTH = 3,
};

typedef GLFWwindow SgWindow;

typedef struct SgApp {
	SgWindow*                 pWindow;
	VkInstance                instance;
#ifdef _DEBUG
	VkDebugUtilsMessengerEXT  debugCallback;
#endif
	VkSurfaceKHR              surface;
	VkPhysicalDevice          physicalDevice;
	// TODO: Do it more generic.
	// With an array of queues and their priorities
	uint32_t                  graphicsQueueFamilyIdx;
	SurfaceAttributes         surfaceAttributes;
	VkDevice                  device;
	VkSampleCountFlags        msaaSampleCount;
	VkQueue                   graphicsQueue;
	VmaAllocator              allocator;

	pthread_t                 threads[SG_THREADS_COUNT];
	// No secondary command buffer pools atm
	VkCommandPool             pCommandPools[SG_FRAME_QUEUE_LENGTH * SG_THREADS_COUNT];

	// Synchorization primitives
	VkSemaphore               pFrameReadySemaphore[SG_FRAME_QUEUE_LENGTH];
	VkSemaphore               pFrameFinishedSemaphore[SG_FRAME_QUEUE_LENGTH];
	uint32_t                  currentFrame;
	uint32_t                  frameImageIndex;
	VkFence                   pFrameFences[SG_FRAME_QUEUE_LENGTH];
	VkFence                   pFrameImageInFlightFences[SG_FRAME_QUEUE_LENGTH + 6];

	VkRect2D                  scissor;
	VkViewport                viewport;
} SgApp;

typedef enum SgResourceTypeFlagBits {
	SG_RESOURCE_TYPE_TEXTURE_2D   = BIT(0),
	SG_RESOURCE_TYPE_MESH         = BIT(1),
	SG_RESOURCE_TYPE_UNIFORM      = BIT(2),
	SG_RESOURCE_TYPE_INDICES      = BIT(3),
	SG_RESOURCE_TYPE_STAGING      = BIT(4), // Hidden from the outer API atm
} SgResourceTypeFlagBits;
typedef SgFlags SgResourceTypeFlags;
static const SgResourceTypeFlags SG_RESOURCE_TYPE_REQIRE_STAGING_MASK = SG_RESOURCE_TYPE_MESH | SG_RESOURCE_TYPE_INDICES;
static const SgResourceTypeFlags SG_RESOURCE_TYPE_IS_IMAGE_MASK = SG_RESOURCE_TYPE_TEXTURE_2D;

typedef struct SgBuffer {
	VkBuffer        buffer;
	void*           bytes;
	uint32_t        size;
	VmaAllocation   allocation;
} SgBuffer;

typedef struct SgImage {
	VkImage             image;
	void*               bytes;
	VkExtent3D          extent;
	uint32_t            size;
	VmaAllocation       allocation;
	VkFormat            format;
} SgImage;

typedef struct SgImageView {
	VkImageView         imageView;
} SgImageView;
typedef enum SgShaderStageFlagBits {
    SG_SHADER_STAGE_VERTEX_BIT   = VK_SHADER_STAGE_VERTEX_BIT,
    SG_SHADER_STAGE_GEOMETRY_BIT = VK_SHADER_STAGE_GEOMETRY_BIT,
    SG_SHADER_STAGE_FRAGMENT_BIT = VK_SHADER_STAGE_FRAGMENT_BIT,
    SG_SHADER_STAGE_COMPUTE_BIT  = VK_SHADER_STAGE_COMPUTE_BIT,
} SgShaderStageFlagBits;
typedef SgFlags SgShaderStageFlags;

typedef struct SgShader {
	VkShaderModule        shader;
	SgShaderStageFlags    stage;
} SgShader;

typedef struct SgShaderPass {
	VkPipeline       pipeline;
	VkPipelineLayout pipelineLayout;
} SgShaderPass;

typedef struct SgResourceBinding {
	SgResourceTypeFlags          type;
	SgShaderStageFlags           stage;
	uint32_t                     binding;
	uint32_t                     setBinding;
} SgResourceBinding;

typedef struct SgSetBindings {
	SgResourceBinding* pBindings;
	uint32_t           bindingCount;
} SgSetBindings;

typedef struct SgResource {
	SgBuffer                     dataBuffer;
	SgImage                      image;
	VkImageView                  imageView;
	VkSampler                    imageSampler;
	SgBuffer                     stagingBuffer;
	SgResourceTypeFlags          type;

	uint32_t*                    pCommandBufferID;

	void*                        lastBytes;
	uint32_t                     lastSize;
	
	const char*                  pName;
} SgResource;

typedef struct SgResourceMap {
	struct hashmap*              pResourceMap;
	VkCommandBuffer*             pCommadBuffers;
	uint32_t                     commandBufferCount;

	VkCommandBuffer*             pCommadBuffersForSpecificUpdate;

	VkFence                      fence;
	VkCommandPool                commandPool;
} SgResourceMap;

typedef struct SgSwapchain {
	VkSwapchainKHR            swapchain;
	VkImage*                  pFrameImages;
	VkImageView*              pFrameImageViews;
	SgImage                   depthImage;
	SgImageView               depthImageView;
	SgImage                   blendImage;
	SgImageView               blendImageView;
	VkFramebuffer*            pFrameBuffers;
	VkExtent2D                extent;
	uint32_t                  imageCount;
} SgSwapchain;

typedef struct SgGraphicsPipelineBuilder {
	VkPipelineShaderStageCreateInfo*       pShaderStages;
	uint32_t                               shaderStageCount;

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
	VkPipelineShaderStageCreateInfo  shaderStage;
	VkPipelineLayout                 pipelineLayout;
} SgComputePipelineBuilder;

typedef struct SgUpdateCommands {
	VkCommandBuffer*             pCommandBuffers;
} SgUpdateCommands;

// Really does look like a file, doesn't it?
typedef struct SgData {
	void*             bytes;
	size_t            size;
} SgData;

typedef struct SgFile {
	uint32_t*         pBytes;
	size_t            size;
} SgFile;

SgResult sgOpenFile(const char* path, SgFile **ppFile);
SgResult sgWriteFile(const char* path, SgFile *pFile);
SgResult sgCloseFile(SgFile **ppFile);

#endif
