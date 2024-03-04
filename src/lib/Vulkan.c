#include "Vulkan.h"

#include <cglm/cglm.h>
#include <string.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "Base.h"

const char* ckp_Vulkan__ERROR_MESSAGES[] = {
    "None\n",
    "volkInitialize() failed.\n",
    "vkCreateInstance() failed.\n",
    "vkEnumerateInstanceLayerProperties() failed to count. RequiredLayerCount: %u\n",
    "vkEnumerateInstanceLayerProperties() count was zero. RequiredLayerCount: %u\n",
    ("vkEnumerateInstanceLayerProperties() failed to read. RequiredLayerCount: %u\n "
     "SupportedLayerCount: %u\n"),
    "vkEnumerateInstanceLayerProperties() missing required layers.\n",
    "vkEnumerateInstanceExtensionProperties() failed to count. RequiredExtensionCount: %u\n",
    "vkEnumerateInstanceExtensionProperties() count was zero. RequiredExtensionCount: %u\n",
    ("vkEnumerateInstanceExtensionProperties() failed to read. RequiredExtensionCount: %u, "
     "SupportedExtensionCount: %u\n"),
    "vkEnumerateInstanceExtensionProperties() missing required extensions.\n",
    "vkEnumeratePhysicalDevices() failed to count.\n",
    "vkEnumeratePhysicalDevices() count was zero.\n",
    "vkEnumeratePhysicalDevices() failed to read. deviceCount: %u\n",
    "Invalid physical device. index: %d\n",
    "vkEnumerateDeviceExtensionProperties() failed to count. RequiredPhysicalExtensionCount: %u\n",
    "vkEnumerateDeviceExtensionProperties() count was zero. RequiredPhysicalExtensionCount: %u\n",
    ("vkEnumerateDeviceExtensionProperties() failed to read. RequiredPhysicalExtensionCount: %u, "
     "SupportedPhysicalExtensionCount: %u\n"),
    "vkEnumerateDeviceExtensionProperties() missing required extensions.\n",
    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed.\n",
    "vkGetPhysicalDeviceSurfaceFormatsKHR() failed to count.\n",
    "vkGetPhysicalDeviceSurfaceFormatsKHR() count was zero.\n",
    ("vkGetPhysicalDeviceSurfaceFormatsKHR() failed to read. formatCount: %u\n"),
    "vkGetPhysicalDeviceSurfacePresentModesKHR() failed to count.\n",
    "vkGetPhysicalDeviceSurfacePresentModesKHR() count was zero.\n",
    "vkGetPhysicalDeviceSurfacePresentModesKHR() failed to read. presentModeCount: %u\n",

};

Vulkan__Error_t Vulkan__InitDriver1(Vulkan_t* self) {
  self->m_requiredDriverExtensionCount = 0;
  self->m_requiredValidationLayerCount = 0;
  self->m_requiredPhysicalDeviceExtensionCount = 0;
  self->m_physicalDevice = VK_NULL_HANDLE;

  self->m_aspectRatio = ASPECT_SQUARE;
  self->m_windowWidth = 0;
  self->m_windowHeight = 0;
  self->m_viewportX = 0;
  self->m_viewportY = 0;
  self->m_viewportWidth = 0;
  self->m_viewportHeight = 0;
  u32 bufferWidth = 0;
  u32 bufferHeight = 0;
  self->m_framebufferResized = false;
  self->m_minimized = false;
  self->m_maximized = false;

  ASSERT_ERROR(
      VK_SUCCESS == volkInitialize(),
      VULKAN_ERROR_VOLK_INITIALIZE_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)

  return VULKAN_ERROR_NONE;
}

static const char* debug_validation_layer1 = "VK_LAYER_KHRONOS_validation";

Vulkan__Error_t Vulkan__AssertDriverValidationLayersSupported(Vulkan_t* self) {
#ifdef DEBUG_VULKAN
  // SDK-provided layer conveniently bundles all useful standard validation
  self->m_requiredValidationLayers[self->m_requiredValidationLayerCount++] =
      debug_validation_layer1;
#endif

  if (self->m_requiredValidationLayerCount > 0) {
    u32 availableLayersCount = 0;
    ASSERT_ERROR(
        VK_SUCCESS == vkEnumerateInstanceLayerProperties(&availableLayersCount, NULL),
        VULKAN_ERROR_VK_EILP_COUNT_FAILED,
        ckp_Vulkan__ERROR_MESSAGES,
        self->m_requiredValidationLayerCount)
    ASSERT_ERROR(
        availableLayersCount > 0,
        VULKAN_ERROR_VK_EILP_COUNT_ZERO,
        ckp_Vulkan__ERROR_MESSAGES,
        self->m_requiredValidationLayerCount)
    LOG_INFOF("Driver Validation Layer Count: %u\n", availableLayersCount);
    VkLayerProperties availableLayers[availableLayersCount];
    ASSERT_ERROR(
        VK_SUCCESS == vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers),
        VULKAN_ERROR_VK_EILP_READ_FAILED,
        ckp_Vulkan__ERROR_MESSAGES,
        self->m_requiredValidationLayerCount,
        availableLayersCount)

    // print list of validation layers to console
    bool found;
    LOG_INFOF("validation layers:\n");
    for (u8 i = 0; i < availableLayersCount; i++) {
      found = false;
      for (u8 i2 = 0; i2 < self->m_requiredValidationLayerCount; i2++) {
        if (0 == strcmp(self->m_requiredValidationLayers[i2], availableLayers[i].layerName)) {
          found = true;
          break;
        }
      }
      LOG_INFOF("  %s%s\n", availableLayers[i].layerName, found ? " (required)" : "");
    }
    // validate the required validation layers are all found
    for (u8 i3 = 0; i3 < self->m_requiredValidationLayerCount; i3++) {
      found = false;
      for (u8 i4 = 0; i4 < availableLayersCount; i4++) {
        if (0 == strcmp(self->m_requiredValidationLayers[i3], availableLayers[i4].layerName)) {
          found = true;
          break;
        }
      }
      if (!found) {
        LOG_INFOF("  missing %s", self->m_requiredValidationLayers[i3]);
        ASSERT_ERROR(found, VULKAN_ERROR_VK_EILP_MISSING_REQUIRED, ckp_Vulkan__ERROR_MESSAGES, NULL)
      }
    }
  }

  return VULKAN_ERROR_NONE;
}

Vulkan__Error_t Vulkan__AssertDriverExtensionsSupported(Vulkan_t* self) {
  // list the extensions supported by this driver
  u32 availableExtensionCount = 0;
  ASSERT_ERROR(
      VK_SUCCESS == vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL),
      VULKAN_ERROR_VK_EIEP_COUNT_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)
  ASSERT_ERROR(
      availableExtensionCount > 0,
      VULKAN_ERROR_VK_EIEP_COUNT_ZERO,
      ckp_Vulkan__ERROR_MESSAGES,
      self->m_requiredDriverExtensionCount)
  LOG_INFOF("Driver Extension Count: %u\n", availableExtensionCount);
  VkExtensionProperties availableExtensions[availableExtensionCount];
  ASSERT_ERROR(
      VK_SUCCESS == vkEnumerateInstanceExtensionProperties(
                        NULL,
                        &availableExtensionCount,
                        availableExtensions),
      VULKAN_ERROR_VK_EIEP_READ_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      self->m_requiredDriverExtensionCount,
      availableExtensionCount)

  // print list of extensions to console
  bool found;
  LOG_INFOF("driver extensions:\n");
  for (u8 i = 0; i < availableExtensionCount; i++) {
    found = false;
    for (u8 i2 = 0; i2 < self->m_requiredDriverExtensionCount; i2++) {
      if (0 == strcmp(self->m_requiredDriverExtensions[i2], availableExtensions[i].extensionName)) {
        found = true;
        break;
      }
    }
    LOG_INFOF("  %s%s\n", availableExtensions[i].extensionName, found ? " (required)" : "");
  }

  // validate the required extensions are all found
  for (u8 i3 = 0; i3 < self->m_requiredDriverExtensionCount; i3++) {
    found = false;
    for (u8 i4 = 0; i4 < availableExtensionCount; i4++) {
      if (0 ==
          strcmp(self->m_requiredDriverExtensions[i3], availableExtensions[i4].extensionName)) {
        found = true;
        break;
      }
    }
    if (!found) {
      LOG_INFOF("  missing %s", self->m_requiredDriverExtensions[i3]);
      ASSERT_ERROR(found, VULKAN_ERROR_VK_EIEP_MISSING_REQUIRED, ckp_Vulkan__ERROR_MESSAGES, NULL)
    }
  }

  return VULKAN_ERROR_NONE;
}

Vulkan__Error_t Vulkan__CreateInstance(
    Vulkan_t* self,
    const char* name,
    const char* engineName,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix) {
  VkInstanceCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = NULL;
#if OS_MAC == 1
  createInfo.flags =
      // enable MoltenVK support for MacOS cross-platform support
      VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
  createInfo.flags = 0;  // default
#endif
  VkApplicationInfo appInfo;
  createInfo.pApplicationInfo = &appInfo;
  {
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, hotfix);
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
  }

  createInfo.enabledLayerCount = self->m_requiredValidationLayerCount;
  createInfo.ppEnabledLayerNames = self->m_requiredValidationLayers;
  createInfo.enabledExtensionCount = self->m_requiredDriverExtensionCount;
  createInfo.ppEnabledExtensionNames = self->m_requiredDriverExtensions;

  ASSERT_ERROR(
      VK_SUCCESS == vkCreateInstance(&createInfo, NULL, &self->m_instance),
      VULKAN_ERROR_VK_CREATE_INSTANCE_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)

  return VULKAN_ERROR_NONE;
}

void Vulkan__InitDriver2(Vulkan_t* self) {
  volkLoadInstance(self->m_instance);
}

void Vulkan__UsePhysicalDevice(Vulkan_t* self, const u8 requiredDeviceIndex) {
  // list GPUs
  u32 deviceCount = 0;
  ASSERT_ERROR(
      VK_SUCCESS == vkEnumeratePhysicalDevices(self->m_instance, &deviceCount, NULL),
      VULKAN_ERROR_VK_EPD_COUNT_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)

  ASSERT_ERROR(deviceCount > 0, VULKAN_ERROR_VK_EPD_COUNT_ZERO, ckp_Vulkan__ERROR_MESSAGES, NULL)

  VkPhysicalDevice devices[deviceCount];
  ASSERT_ERROR(
      VK_SUCCESS == vkEnumeratePhysicalDevices(self->m_instance, &deviceCount, devices),
      VULKAN_ERROR_VK_EPD_READ_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      deviceCount)

  // print all GPUs found
  LOG_INFOF("devices:\n");
  for (u8 i = 0; i < deviceCount; i++) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);

    // assumption: we only want the type of GPU device used to render video games
    const bool discrete = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    const bool geometry = deviceFeatures.geometryShader;

    // list each GPU device found
    LOG_INFOF(
        "  %u: %s%s%s%s\n",
        i,
        deviceProperties.deviceName,
        i == requiredDeviceIndex ? " (selected)" : "",
        discrete ? " DISCRETE" : "",
        geometry ? " GEOMETRY_SHADER" : "");

    // select one GPU to be the active/default/current for all subsequent Vulkan methods;
    // it must meet certain minimum requirements
    if (i == requiredDeviceIndex /*&& discrete*/ /*&& geometry*/) {
      self->m_physicalDevice = devices[i];
      return;
    }
  }

  ASSERT_ERROR(
      false,
      VULKAN_ERROR_INVALID_DEVICE_IDX,
      ckp_Vulkan__ERROR_MESSAGES,
      requiredDeviceIndex)
}

static const char* specialPhysicalExtension1 = "VK_KHR_portability_subset";

void Vulkan__AssertSwapChainSupported(Vulkan_t* self) {
  self->m_requiredPhysicalDeviceExtensions[self->m_requiredPhysicalDeviceExtensionCount++] =
      VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  // list the extensions supported by this physical device
  u32 availablePhysicalExtensionCount = 0;
  ASSERT_ERROR(
      VK_SUCCESS == vkEnumerateDeviceExtensionProperties(
                        self->m_physicalDevice,
                        NULL,
                        &availablePhysicalExtensionCount,
                        NULL),
      VULKAN_ERROR_VK_EDEP_COUNT_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      self->m_requiredPhysicalDeviceExtensionCount)
  ASSERT_ERROR(
      availablePhysicalExtensionCount > 0,
      VULKAN_ERROR_VK_EDEP_COUNT_ZERO,
      ckp_Vulkan__ERROR_MESSAGES,
      self->m_requiredPhysicalDeviceExtensionCount)
  LOG_INFOF("Physical Device Extension Count: %u\n", availablePhysicalExtensionCount);
  VkExtensionProperties availablePhysicalExtensions[availablePhysicalExtensionCount];
  ASSERT_ERROR(
      VK_SUCCESS == vkEnumerateDeviceExtensionProperties(
                        self->m_physicalDevice,
                        NULL,
                        &availablePhysicalExtensionCount,
                        availablePhysicalExtensions),
      VULKAN_ERROR_VK_EDEP_READ_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      self->m_requiredPhysicalDeviceExtensionCount,
      availablePhysicalExtensionCount)

  // quirk: vulkan spec says if this is supported, we must request it
  for (u8 i = 0; i < self->m_requiredPhysicalDeviceExtensionCount; i++) {
    if (0 == strcmp(specialPhysicalExtension1, self->m_requiredPhysicalDeviceExtensions[i])) {
      self->m_requiredPhysicalDeviceExtensions[self->m_requiredPhysicalDeviceExtensionCount++] =
          specialPhysicalExtension1;
    }
  }

  // print list of extensions to console
  LOG_INFOF("required device extensions:\n")
  // validate the required extensions are all found
  bool found = false;
  for (u8 i2 = 0; i2 < self->m_requiredPhysicalDeviceExtensionCount; i2++) {
    found = false;
    for (u8 i3 = 0; i3 < availablePhysicalExtensionCount; i3++) {
      if (0 == strcmp(
                   self->m_requiredPhysicalDeviceExtensions[i2],
                   availablePhysicalExtensions[i3].extensionName)) {
        found = true;
        break;
      }
    }

    LOG_INFOF(
        "  %s%s\n",
        self->m_requiredPhysicalDeviceExtensions[i2],
        found ? " (required)" : " (missing)")

    ASSERT_ERROR(
        found,
        VULKAN_ERROR_VK_EDEP_MISSING_REQUIRED,
        ckp_Vulkan__ERROR_MESSAGES,
        self->m_requiredPhysicalDeviceExtensionCount,
        availablePhysicalExtensionCount)
  }

  ASSERT_ERROR(
      VK_SUCCESS == vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &self->m_SwapChain__capabilities),
      VULKAN_ERROR_VK_GPDSC_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)

  u32 availableFormatCount = 0;
  ASSERT_ERROR(
      VK_SUCCESS == vkGetPhysicalDeviceSurfaceFormatsKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &availableFormatCount,
                        NULL),
      VULKAN_ERROR_VK_GPDSF_COUNT_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)
  ASSERT_ERROR(
      availableFormatCount > 0,
      VULKAN_ERROR_VK_GPDSF_COUNT_ZERO,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)
  LOG_INFOF("Physical Device Surface Format Count: %u\n", availableFormatCount);
  ASSERT_ERROR(
      VK_SUCCESS == vkGetPhysicalDeviceSurfaceFormatsKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &availableFormatCount,
                        self->m_SwapChain__formats),
      VULKAN_ERROR_VK_GPDSF_READ_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      availableFormatCount)

  u32 availablePresentModeCount = 0;
  ASSERT_ERROR(
      VK_SUCCESS == vkGetPhysicalDeviceSurfacePresentModesKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &availablePresentModeCount,
                        NULL),
      VULKAN_ERROR_VK_GPDSPM_COUNT_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)
  ASSERT_ERROR(
      availablePresentModeCount > 0,
      VULKAN_ERROR_VK_GPDSPM_COUNT_ZERO,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL)
  LOG_INFOF("Physical Device Surface Present Mode Count: %u\n", availablePresentModeCount);
  ASSERT_ERROR(
      VK_SUCCESS == vkGetPhysicalDeviceSurfacePresentModesKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &availablePresentModeCount,
                        self->m_SwapChain__presentModes),
      VULKAN_ERROR_VK_GPDSPM_READ_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      availablePresentModeCount)
}

void Vulkan__UseLogicalDevice(Vulkan_t* self) {
}

void Vulkan__CreateSwapChain(Vulkan_t* self) {
}