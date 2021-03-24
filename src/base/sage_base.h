#ifndef SAGE_BASE
#define SAGE_BASE
//#define _DEBUG

#include <stddef.h>
#include <stdint.h>
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vk_mem_alloc.h"
// TODO: Thread wrapper to easily change out apis when needed
#include <pthread.h>

#define SG_DEFINE_HANDLE( object ) typedef struct object##_T *object;
#define NUMOF(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BIT(n)     (1UL << (n))
#define SG_CALLOC_NUM(object, num) (object) = (calloc((num), sizeof((*object))));
#define SG_MALLOC_NUM(object, num) (object) = (malloc((num) * sizeof((*object))));

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

typedef struct SgResource {
	SgResourceTypeFlags          type;
	SgShaderStageFlags           stage;
	SgBuffer                     dataBuffer;
	SgImage                      image;
	VkImageView                  imageView;
	VkSampler                    imageSampler;
	SgBuffer                     stagingBuffer;
	VkCommandBuffer              commandBuffer;
	VkCommandPool                commandPool;
	uint32_t                     binding;
} SgResource;

typedef struct SgResourceSet {
	SgResource**                     ppResources;
	uint32_t                         resourceCount;
	uint32_t                         setIndex;
	VkDescriptorSetLayoutBinding*    pSetLayoutBindings;
	VkDescriptorSetLayout            setLayout;
	VkWriteDescriptorSet*            pWriteDescriptorSets;
} SgResourceSet;

typedef struct SgShader {
	VkShaderModule        shader;
	SgShaderStageFlags    stage;
} SgShader;

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

typedef struct SgGraphicsInstance {
	VkRenderPass                   renderPass;

	SgResourceSet**                ppSets;
	uint32_t                       setCount;
	VkDescriptorSetLayout*         pDescriptorSetLayouts;
	VkDescriptorPool               descriptorPool;
	VkDescriptorSet**              ppDescriptorSets;
	uint32_t                       descriptorSetsCount;

	VkPipelineLayout               pipelineLayout;
	SgSwapchain                    swapchain;
	VkPipeline                     graphicsPipeline;
} SgGraphicsInstance;

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
