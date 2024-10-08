set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Window)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Window)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp" "${SRCROOT}/Stub/*.hpp" "${SRCROOT}/Stub/*.cpp")

# add platform specific sources
if(SFML_OS_WINDOWS)
    set(PLATFORM_SRC
        ${SRCROOT}/Win32/ClipboardImpl.hpp
        ${SRCROOT}/Win32/CursorImpl.hpp
        ${SRCROOT}/Win32/JoystickImpl.hpp
        ${SRCROOT}/Win32/Utils.hpp
        ${SRCROOT}/Win32/WindowImplWin32.hpp

        ${SRCROOT}/Win32/Win32ImplUnity.cpp

        # ${SRCROOT}/Win32/Unity/ClipboardImpl.cpp
        # ${SRCROOT}/Win32/Unity/CursorImpl.cpp
        # ${SRCROOT}/Win32/Unity/InputImpl.cpp
        # ${SRCROOT}/Win32/Unity/JoystickImpl.cpp
        # ${SRCROOT}/Win32/Unity/Utils.cpp
        # ${SRCROOT}/Win32/Unity/VideoModeImpl.cpp
        # ${SRCROOT}/Win32/Unity/VulkanImplWin32.cpp
        # ${SRCROOT}/Win32/Unity/WindowImplWin32.cpp
    )
    if(SFML_OPENGL_ES)
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/EGL/EGLCheck.cpp
            ${SRCROOT}/EGL/EGLCheck.hpp
            ${SRCROOT}/EGL/EGLContext.cpp
            ${SRCROOT}/EGL/EGLContext.hpp
            ${SRCROOT}/EGL/EGLGlad.cpp
            ${SRCROOT}/EGL/EGLGlad.hpp
        )
    else()
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/Win32/WglContext.cpp
            ${SRCROOT}/Win32/WglContext.hpp
        )
    endif()
elseif(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
    if(SFML_USE_DRM)
        add_definitions(-DSFML_USE_DRM)
        set(PLATFORM_SRC
            ${SRCROOT}/EGL/EGLCheck.cpp
            ${SRCROOT}/EGL/EGLCheck.hpp
            ${SRCROOT}/DRM/InputImpl.cpp
            ${SRCROOT}/DRM/VideoModeImpl.cpp
            ${SRCROOT}/DRM/DRMContext.cpp
            ${SRCROOT}/DRM/DRMContext.hpp
            ${SRCROOT}/DRM/WindowImplDRM.cpp
            ${SRCROOT}/DRM/WindowImplDRM.hpp
        )
    else()
        set(PLATFORM_SRC
            ${SRCROOT}/Unix/CursorImpl.hpp
            ${SRCROOT}/Unix/CursorImpl.cpp
            ${SRCROOT}/Unix/ClipboardImpl.hpp
            ${SRCROOT}/Unix/ClipboardImpl.cpp
            ${SRCROOT}/Unix/InputImpl.cpp
            ${SRCROOT}/Unix/KeyboardImpl.hpp
            ${SRCROOT}/Unix/KeyboardImpl.cpp
            ${SRCROOT}/Unix/KeySymToKeyMapping.hpp
            ${SRCROOT}/Unix/KeySymToKeyMapping.cpp
            ${SRCROOT}/Unix/KeySymToUnicodeMapping.hpp
            ${SRCROOT}/Unix/KeySymToUnicodeMapping.cpp
            ${SRCROOT}/Unix/Display.cpp
            ${SRCROOT}/Unix/Display.hpp
            ${SRCROOT}/Unix/Utils.hpp
            ${SRCROOT}/Unix/VideoModeImpl.cpp
            ${SRCROOT}/Unix/VulkanImplX11.cpp
            ${SRCROOT}/Unix/WindowImplX11.cpp
            ${SRCROOT}/Unix/WindowImplX11.hpp
        )
        if(SFML_OPENGL_ES)
            list(APPEND PLATFORM_SRC
                ${SRCROOT}/EGL/EGLCheck.cpp
                ${SRCROOT}/EGL/EGLCheck.hpp
                ${SRCROOT}/EGL/EGLContext.cpp
                ${SRCROOT}/EGL/EGLContext.hpp
            )
        else()
             list(APPEND PLATFORM_SRC
                ${SRCROOT}/Unix/GlxContext.cpp
                ${SRCROOT}/Unix/GlxContext.hpp
            )
        endif()
    endif()
    if(SFML_OS_LINUX)
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/Unix/JoystickImpl.cpp
            ${SRCROOT}/Unix/JoystickImpl.hpp
        )
    elseif(SFML_OS_FREEBSD)
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/FreeBSD/JoystickImpl.cpp
            ${SRCROOT}/FreeBSD/JoystickImpl.hpp
        )
    elseif(SFML_OS_NETBSD)
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/NetBSD/JoystickImpl.cpp
            ${SRCROOT}/NetBSD/JoystickImpl.hpp
        )
    endif()
elseif(SFML_OS_MACOS)
    enable_language(OBJC OBJCXX)
    set(PLATFORM_SRC
        ${SRCROOT}/macOS/cg_sf_conversion.hpp
        ${SRCROOT}/macOS/cg_sf_conversion.mm
        ${SRCROOT}/macOS/CursorImpl.hpp
        ${SRCROOT}/macOS/CursorImpl.mm
        ${SRCROOT}/macOS/ClipboardImpl.hpp
        ${SRCROOT}/macOS/ClipboardImpl.mm
        ${SRCROOT}/macOS/InputImpl.mm
        ${SRCROOT}/macOS/HIDInputManager.hpp
        ${SRCROOT}/macOS/HIDInputManager.mm
        ${SRCROOT}/macOS/HIDJoystickManager.hpp
        ${SRCROOT}/macOS/HIDJoystickManager.cpp
        ${SRCROOT}/macOS/JoystickImpl.hpp
        ${SRCROOT}/macOS/NSImage+raw.h
        ${SRCROOT}/macOS/NSImage+raw.mm
        ${SRCROOT}/macOS/Scaling.h
        ${SRCROOT}/macOS/SensorImpl.cpp
        ${SRCROOT}/macOS/SensorImpl.hpp
        ${SRCROOT}/macOS/SFApplication.h
        ${SRCROOT}/macOS/SFApplication.m
        ${SRCROOT}/macOS/SFApplicationDelegate.h
        ${SRCROOT}/macOS/SFApplicationDelegate.m
        ${SRCROOT}/macOS/SFContext.hpp
        ${SRCROOT}/macOS/SFContext.mm
        ${SRCROOT}/macOS/SFKeyboardModifiersHelper.h
        ${SRCROOT}/macOS/SFKeyboardModifiersHelper.mm
        ${SRCROOT}/macOS/SFOpenGLView.h
        ${SRCROOT}/macOS/SFOpenGLView.mm
        ${SRCROOT}/macOS/SFOpenGLView+keyboard.mm
        ${SRCROOT}/macOS/SFOpenGLView+keyboard_priv.h
        ${SRCROOT}/macOS/SFOpenGLView+mouse.mm
        ${SRCROOT}/macOS/SFOpenGLView+mouse_priv.h
        ${SRCROOT}/macOS/SFSilentResponder.h
        ${SRCROOT}/macOS/SFSilentResponder.m
        ${SRCROOT}/macOS/SFWindow.h
        ${SRCROOT}/macOS/SFWindow.m
        ${SRCROOT}/macOS/SFWindowController.h
        ${SRCROOT}/macOS/SFWindowController.mm
        ${SRCROOT}/macOS/SFViewController.h
        ${SRCROOT}/macOS/SFViewController.mm
        ${SRCROOT}/macOS/VideoModeImpl.cpp
        ${SRCROOT}/macOS/WindowImplCocoa.hpp
        ${SRCROOT}/macOS/WindowImplCocoa.mm
        ${SRCROOT}/macOS/WindowImplDelegateProtocol.h
        ${SRCROOT}/macOS/AutoreleasePoolWrapper.hpp
        ${SRCROOT}/macOS/AutoreleasePoolWrapper.mm
    )
elseif(SFML_OS_IOS)
    enable_language(OBJCXX)
    set(PLATFORM_SRC
        ${SRCROOT}/iOS/ClipboardImpl.mm
        ${SRCROOT}/iOS/ClipboardImpl.hpp
        ${SRCROOT}/iOS/EaglContext.mm
        ${SRCROOT}/iOS/EaglContext.hpp
        ${SRCROOT}/iOS/InputImpl.mm
        ${SRCROOT}/iOS/SensorImpl.mm
        ${SRCROOT}/iOS/SensorImpl.hpp
        ${SRCROOT}/iOS/VideoModeImpl.mm
        ${SRCROOT}/iOS/WindowImplUIKit.hpp
        ${SRCROOT}/iOS/WindowImplUIKit.mm
        ${SRCROOT}/iOS/ObjCType.hpp
        ${SRCROOT}/iOS/SFAppDelegate.hpp
        ${SRCROOT}/iOS/SFAppDelegate.mm
        ${SRCROOT}/iOS/SFView.hpp
        ${SRCROOT}/iOS/SFView.mm
        ${SRCROOT}/iOS/SFViewController.hpp
        ${SRCROOT}/iOS/SFViewController.mm
        ${SRCROOT}/iOS/SFMain.hpp
        ${SRCROOT}/iOS/SFMain.mm
    )
elseif(SFML_OS_ANDROID)
    set(PLATFORM_SRC
        ${SRCROOT}/EGL/EGLCheck.cpp
        ${SRCROOT}/EGL/EGLCheck.hpp
        ${SRCROOT}/EGL/EGLContext.cpp
        ${SRCROOT}/EGL/EGLContext.hpp
        ${SRCROOT}/Android/WindowImplAndroid.hpp
        ${SRCROOT}/Android/WindowImplAndroid.cpp
        ${SRCROOT}/Android/VideoModeImpl.cpp
        ${SRCROOT}/Android/InputImpl.cpp
        ${SRCROOT}/Android/SensorImpl.hpp
        ${SRCROOT}/Android/SensorImpl.cpp
    )
elseif(SFML_OS_EMSCRIPTEN)
    set(PLATFORM_SRC
        ${SRCROOT}/EGL/EGLCheck.cpp
        ${SRCROOT}/EGL/EGLCheck.hpp
        ${SRCROOT}/EGL/EGLContext.cpp
        ${SRCROOT}/EGL/EGLContext.hpp
        ${SRCROOT}/Emscripten/WindowImplEmscripten.hpp
        ${SRCROOT}/Emscripten/JoystickImpl.hpp
        ${SRCROOT}/Emscripten/EmscriptenImpl.cpp
    )
endif()

# define the sfml-window target
sfml_add_library(Window
                 SOURCES ${SRC} ${PLATFORM_SRC})
# DRM libraries
if(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
    if(SFML_USE_DRM)
        find_package(DRM REQUIRED)
        find_package(GBM REQUIRED)
        target_link_libraries(sfml-window PRIVATE DRM::DRM GBM::GBM)
    else()
        find_package(X11 REQUIRED COMPONENTS Xrandr Xcursor Xi)
        target_link_libraries(sfml-window PRIVATE X11::X11 X11::Xrandr X11::Xcursor X11::Xi)
    endif()
endif()
target_link_libraries(sfml-window PUBLIC SFML::System)

# glad sources
target_include_directories(sfml-window SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

# When static linking on macOS, we need to add this flag for objective C to work
# https://developer.apple.com/library/archive/qa/qa1490/_index.html
if((NOT BUILD_SHARED_LIBS) AND SFML_OS_MACOS)
    target_link_libraries(sfml-window PRIVATE -ObjC)
endif()

# Vulkan headers
target_include_directories(sfml-window SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/vulkan")

# CMake 3.11 and later prefer to choose GLVND, but we choose legacy OpenGL for backward compatibility
# (unless the OpenGL_GL_PREFERENCE was explicitly set)
# See CMP0072 for more details (cmake --help-policy CMP0072)
if(NOT OpenGL_GL_PREFERENCE)
    set(OpenGL_GL_PREFERENCE "LEGACY")
endif()

if(SFML_OS_IOS)
    target_link_libraries(sfml-window PRIVATE "-framework OpenGLES")
elseif(SFML_OS_ANDROID OR SFML_OS_EMSCRIPTEN OR SFML_OPENGL_ES)
    find_package(EGL REQUIRED)
    find_package(GLES REQUIRED)
    target_link_libraries(sfml-window PRIVATE EGL::EGL GLES::GLES)
else()
    find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    target_link_libraries(sfml-window PRIVATE OpenGL::GL)
endif()

if(SFML_OS_WINDOWS AND NOT SFML_COMPILER_MSVC)
    include(CheckIncludeFileCXX)
    check_include_file_cxx(dinput.h DINPUT_H_FOUND)
    if(NOT DINPUT_H_FOUND)
        target_include_directories(sfml-window SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/mingw")
    endif()
endif()

if(SFML_OS_LINUX)
    find_package(UDev REQUIRED)
    target_link_libraries(sfml-window PRIVATE UDev::UDev dl)
elseif(SFML_OS_WINDOWS)
    target_link_libraries(sfml-window PRIVATE winmm gdi32)
elseif(SFML_OS_FREEBSD)
    target_link_libraries(sfml-window PRIVATE usbhid)
elseif(SFML_OS_MACOS)
    target_link_libraries(sfml-window PRIVATE "-framework Foundation" "-framework AppKit" "-framework IOKit" "-framework Carbon")
elseif(SFML_OS_IOS)
    target_link_libraries(sfml-window PUBLIC "-framework Foundation" "-framework UIKit" "-framework CoreGraphics" "-framework QuartzCore" "-framework CoreMotion")
elseif(SFML_OS_ANDROID)
    target_link_libraries(sfml-window PRIVATE android)
endif()

# on some platforms (e.g. Raspberry Pi 3 armhf), GCC requires linking libatomic to use <atomic> features
# that aren't supported by native CPU instructions (64-bit atomic operations on 32-bit architecture)
if(SFML_COMPILER_GCC)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("#include <atomic>
        int main(){std::atomic_ullong x(1); return x.fetch_add(1);}" ATOMIC_TEST)
    if(NOT ATOMIC_TEST)
        target_link_libraries(sfml-window PRIVATE atomic)
    endif()
endif()
