#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Vulkan.hpp"

#include "SFML/Base/Assert.hpp"

#ifdef SFML_SYSTEM_WINDOWS

#include "SFML/Window/VulkanImpl.hpp"

#elif defined(SFML_SYSTEM_LINUX_OR_BSD)

#ifdef SFML_USE_DRM
#define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE
#else
#include "SFML/Window/VulkanImpl.hpp"
#endif

#else // not Windows nor Unix

#define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#endif


namespace sf
{
////////////////////////////////////////////////////////////
bool Vulkan::isAvailable([[maybe_unused]] bool requireGraphics)
{
#ifdef SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

    return false;

#else

    return priv::VulkanImpl::isAvailable(requireGraphics);

#endif
}


////////////////////////////////////////////////////////////
VulkanFunctionPointer Vulkan::getFunction([[maybe_unused]] const char* name)
{
    SFML_BASE_ASSERT(name != nullptr && "Name cannot be a null pointer");

#ifdef SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

    return nullptr;

#else

    return priv::VulkanImpl::getFunction(name);

#endif
}


////////////////////////////////////////////////////////////
const std::vector<const char*>& Vulkan::getGraphicsRequiredInstanceExtensions()
{
#ifdef SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

    static const std::vector<const char*> empty;
    return empty;

#else

    return priv::VulkanImpl::getGraphicsRequiredInstanceExtensions();

#endif
}

} // namespace sf
