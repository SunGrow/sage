#include "sg_rend.h"
#ifdef _DEBUG
#include "debug/sg_rend_debug.h"
#endif
#include "device/sg_rend_device.h"
#include "surface/sg_rend_surface.h"
#include <stdlib.h>
#include "log.h"

static const char* SG_SURF_EXT[] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _DEBUG
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
};

SgResult createGLFWwindow(const SgAppCreateInfo *pCreateInfo, SgApp *pApp) {
	if (pCreateInfo->flags & SG_APP_WINDOW_RESIZABLE) {
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (pCreateInfo->flags & SG_APP_WINDOW_FULLSCREEN) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		 
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		pApp->pWindow = glfwCreateWindow(mode->width, mode->height, pCreateInfo->pName, monitor, NULL);
	} else {
		pApp->pWindow = glfwCreateWindow(pCreateInfo->size[0], pCreateInfo->size[1], pCreateInfo->pName, NULL, NULL);
	}
	if(pApp->pWindow) {
		log_info("[AppInit]: GLFW window created");
	} else {
		log_fatal("[AppInit]: GLFW window creation error");
	}

	if (pCreateInfo->flags & SG_APP_CURSOR_HIDDEN) {
		glfwSetInputMode(pApp->pWindow , GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	return SG_SUCCESS;
}

SgResult createVkInstance(const SgAppCreateInfo *pCreateInfo, SgApp *pApp) {
	/* Instance */
    uint32_t apiver = volkGetInstanceVersion();
    if (apiver >= VK_API_VERSION_1_2) {
		log_info("[AppInit]: Vulkan API version 1.2 of newer found");
	} else {
		log_error("[AppInit]: Vulkan API version is too old");
	}
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = pCreateInfo->pName,
        .pEngineName = "svkr",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_API_VERSION_1_2,
    };

    uint32_t glfwExtCount = 0;
    const char **glfwExt;
    glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    uint32_t extCount = NUMOF(SG_SURF_EXT) + glfwExtCount;
    const char *pExt[extCount]; // using vla
    for (uint32_t i = 0; i < glfwExtCount; ++i) {
        pExt[i] = glfwExt[i];
    }
    for (uint32_t i = glfwExtCount; i < extCount; ++i) {
        pExt[i] = SG_SURF_EXT[i - glfwExtCount];
    }

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .ppEnabledExtensionNames = pExt,
        .enabledExtensionCount = extCount,
    };
#ifdef _DEBUG
    const char *pdeblayers[] = {
        "VK_LAYER_KHRONOS_validation",
    };
    createInfo.ppEnabledLayerNames = pdeblayers;
    createInfo.enabledLayerCount = NUMOF(pdeblayers);
#endif
    VkResult vkRes = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &pApp->instance);
	if (vkRes == VK_SUCCESS) {
		log_info("[AppInit]: Vulkan Instance created");
	} else { 
		log_fatal("[AppInit]: Vulkan Instance creation failure");
	}

	volkLoadInstance(pApp->instance);

	return SG_SUCCESS;
}

SgResult sgCreateApp(const SgAppCreateInfo *pCreateInfo, SgApp **ppSgApp) {
	/* TODO: Implement custom allocation callbacks */
	// static could be an alternative... But it is not thread safe. Mb will change to static later.
	SgApp* pApp = calloc(1,sizeof(SgApp));

	glfwInit();
	volkInitialize();

	/* Window */
	createGLFWwindow(pCreateInfo, pApp);
	/* Vulkan Instance */
	createVkInstance(pCreateInfo, pApp);
#ifdef _DEBUG
	/* Vulkan Debug Callback */
	registerDebugCallback(pApp);
#endif
	/* Vulkan Surface */
	createWindowSurface(pApp);
	/* Vulkan Physical Device */
	getPhysicalDevice(pApp);
	/* Vulkan Logical Device */
	getLogicalDevice(pApp);

	/* Vulkan Surface Attirbutes */
	getSurfaceAttributes(pApp);


	/* End */
	*ppSgApp = pApp;

	return SG_SUCCESS;
}

SgBool sgAppUpdate(const SgApp** ppSgApp) {
	if(glfwWindowShouldClose((*ppSgApp)->pWindow))
		return 0;
	glfwPollEvents();

	return 1;
}

