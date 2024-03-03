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
};

Vulkan__Error_t Vulkan__InitDriver(Vulkan_t* self) {
  self->m_requiredDriverExtensionCount = 0;
  self->m_requiredDriverExtensions = NULL;
  self->m_requiredValidationLayerCount = 0;
  self->m_requiredValidationLayers = NULL;
  self->m_instance = NULL;

  VkResult r = volkInitialize();
  if (VK_SUCCESS != r) {
    return VULKAN_ERROR_VOLK_INITIALIZE_FAILED;
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
  VkApplicationInfo appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.pApplicationName = name;
  appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, hotfix);
  appInfo.pEngineName = engineName;
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

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
  createInfo.pApplicationInfo = &appInfo;

#ifdef DEBUG_VULKAN
  // the number of global layers to enable.
  createInfo.enabledLayerCount = self->m_requiredValidationLayerCount;
  createInfo.ppEnabledLayerNames = self->m_requiredValidationLayers;
#else
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = NULL;
#endif
  createInfo.enabledExtensionCount = self->m_requiredDriverExtensionCount;
  createInfo.ppEnabledExtensionNames = self->m_requiredDriverExtensions;

  if (vkCreateInstance(&createInfo, NULL, self->m_instance) != VK_SUCCESS) {
    return VULKAN_ERROR_VK_CREATE_INSTANCE_FAILED;
  }

  return VULKAN_ERROR_NONE;
}