#ifndef SAGE_BASE
#define SAGE_BASE

#include <stddef.h>
#include <stdint.h>
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vk_mem_alloc.h"

#define SG_DEFINE_HANDLE( object ) typedef struct object##_T *object;
#define NUMOF(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef enum {
	SG_SUCCESS = 0,
} SgResult;

typedef struct SurfaceAttributes {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR presentMode;
} SurfaceAttributes;

typedef struct SgApp {
	GLFWwindow*               pWindow;
	VkInstance                instance;
#ifdef _DEBUG
	VkDebugReportCallbackEXT  debugCallback;
#endif
	VkSurfaceKHR              surface;
	VkPhysicalDevice          physicalDevice;
	// TODO: Do it more generic.
	// With an array of queues and their priorities
	uint32_t                  graphicsQueueFamilyIdx;
	SurfaceAttributes         surfaceAttributes;
	VkDevice                  device;
	VmaAllocator              allocator;
} SgApp;


typedef uint32_t SgBool;
typedef uint32_t SgFlags;

#endif
