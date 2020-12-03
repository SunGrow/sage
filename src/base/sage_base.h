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

#define SG_DEFINE_HANDLE( object ) typedef struct object##_T *object;
#define NUMOF(arr) (sizeof(arr) / sizeof((arr)[0]))

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

typedef struct SgApp {
	GLFWwindow*               pWindow;
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
	VkFence                   pFrameImageInFlightFences[SG_FRAME_QUEUE_LENGTH];
} SgApp;

typedef enum SgResourceTypeFlagBits {
	SG_RESOURCE_TYPE_TEXTURE_2D   = 0x00000001,
	SG_RESOURCE_TYPE_MESH         = 0x00000002,
	SG_RESOURCE_TYPE_UNIFORM      = 0x00000004,
	SG_RESOURCE_TYPE_INDICES      = 0x00000008,
	SG_RESOURCE_TYPE_STAGING      = 0x00000010, // Hidden from the outer API atm
} SgResourceTypeFlagBits;
typedef SgFlags SgResourceTypeFlags;

typedef struct SgBuffer {
	VkBuffer        buffer;
	void*           bytes;
	uint32_t        size;
	VmaAllocation   allocation;
} SgBuffer;

typedef struct SgImage {
	VkImage             image;
	void*               bytes;
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
	uint32_t                     binding;
} SgResource;

typedef struct SgResourceSet {
	SgResource*                      pResources;
	uint32_t                         resourceCount;
	uint32_t                         setIndex;
	VkDescriptorSetLayoutBinding*    pSetLayoutBindings;
	VkDescriptorSetLayout            setLayout;
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
	VkDescriptorSet*               pDescriptorSets;

	VkPipelineLayout               pipelineLayout;
	SgSwapchain                    swapchain;
	VkPipeline                     graphicsPipeline;
} SgGraphicsInstance;

typedef struct SgUpdateCommands {
	VkCommandBuffer             pCommandBuffers[SG_FRAME_QUEUE_LENGTH];
	SgResource**           ppIndexResources;
	uint32_t               indexResourceCount;
} SgUpdateCommands;

typedef struct SgFile {
	uint32_t*         pBytes;
	size_t            size;
} SgFile;

SgResult sgOpenFile(const char* path, SgFile **ppFile);
SgResult sgCloseFile(SgFile **ppFile);

#endif
