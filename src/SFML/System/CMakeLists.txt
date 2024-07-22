set(BASE_INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Base)
set(BASE_SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Base)

# all source files
set(SRC_BASE
    # headers
    ${BASE_INCROOT}/Algorithm.hpp
    ${BASE_INCROOT}/Assert.hpp
    ${BASE_INCROOT}/EnumArray.hpp
    ${BASE_INCROOT}/EnumClassBitwiseOps.hpp
    ${BASE_INCROOT}/FixedFunction.hpp
    ${BASE_INCROOT}/IndexSequence.hpp
    ${BASE_INCROOT}/InPlacePImpl.hpp
    ${BASE_INCROOT}/Launder.hpp
    ${BASE_INCROOT}/Macros.hpp
    ${BASE_INCROOT}/MakeIndexSequence.hpp
    ${BASE_INCROOT}/Math.hpp
    ${BASE_INCROOT}/MaxAlignT.hpp
    ${BASE_INCROOT}/Optional.hpp
    ${BASE_INCROOT}/OverloadSet.hpp
    ${BASE_INCROOT}/PassKey.hpp
    ${BASE_INCROOT}/PlacementNew.hpp
    ${BASE_INCROOT}/SizeT.hpp
    ${BASE_INCROOT}/Traits/IsBaseOf.hpp
    ${BASE_INCROOT}/Traits/IsEnum.hpp
    ${BASE_INCROOT}/Traits/IsFloatingPoint.hpp
    ${BASE_INCROOT}/Traits/IsSame.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyCopyable.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyCopyAssignable.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyCopyConstructible.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyDestructible.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyMoveAssignable.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyMoveConstructible.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyMoveConstructible.hpp
    ${BASE_INCROOT}/Traits/RemoveCVRef.hpp
    ${BASE_INCROOT}/Traits/RemoveReference.hpp
    ${BASE_INCROOT}/Traits/UnderlyingType.hpp
    ${BASE_INCROOT}/TypePackElement.hpp
    ${BASE_INCROOT}/UniquePtr.hpp
    ${BASE_INCROOT}/Variant.hpp

    # sources
    ${BASE_SRCROOT}/Assert.cpp
    ${BASE_SRCROOT}/Optional.cpp
)





set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/System)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/System)

# all source files
set(SRC
    ${INCROOT}/Angle.hpp
    ${INCROOT}/Angle.inl
    ${SRCROOT}/Clock.cpp
    ${INCROOT}/Clock.hpp
    ${SRCROOT}/Err.cpp
    ${INCROOT}/Err.hpp
    ${INCROOT}/Export.hpp
    ${INCROOT}/InputStream.hpp
    ${INCROOT}/NativeActivity.hpp
    ${SRCROOT}/Sleep.cpp
    ${INCROOT}/Sleep.hpp
    ${SRCROOT}/String.cpp
    ${INCROOT}/String.hpp
    ${INCROOT}/StringUtfUtils.hpp
    ${INCROOT}/StringUtfUtils.inl
    ${INCROOT}/Time.hpp
    ${INCROOT}/Time.inl
    ${INCROOT}/TimeChronoUtil.hpp
    ${INCROOT}/TimeChronoUtil.inl
    ${INCROOT}/Utf.hpp
    ${INCROOT}/Utf.inl
    ${SRCROOT}/Utils.hpp
    ${SRCROOT}/Utils.cpp
    ${SRCROOT}/Vector2.cpp
    ${INCROOT}/Vector2.hpp
    ${INCROOT}/Vector2.inl
    ${SRCROOT}/Vector3.cpp
    ${INCROOT}/Vector3.hpp
    ${INCROOT}/Vector3.inl
    ${SRCROOT}/FileInputStream.cpp
    ${INCROOT}/FileInputStream.hpp
    ${SRCROOT}/MemoryInputStream.cpp
    ${INCROOT}/MemoryInputStream.hpp
    ${INCROOT}/SuspendAwareClock.hpp
    ${SRCROOT}/LifetimeDependee.cpp
    ${INCROOT}/LifetimeDependee.hpp
    ${SRCROOT}/LifetimeDependant.cpp
    ${INCROOT}/LifetimeDependant.hpp
    ${SRCROOT}/StringUtils.hpp
    ${SRCROOT}/StringUtils.cpp
    ${INCROOT}/Path.hpp
    ${SRCROOT}/Path.cpp
    ${SRCROOT}/PathUtils.hpp
    ${SRCROOT}/Rect.cpp
    ${INCROOT}/Rect.hpp
    ${INCROOT}/Rect.inl
    ${INCROOT}/RectUtils.hpp
    ${INCROOT}/RectUtils.inl
)

list(APPEND SRC ${SRC_BASE})

if(SFML_ENABLE_PCH)
    list(APPEND SRC ${SRCROOT}/PCH.cpp)
endif()

source_group("" FILES ${SRC})

# add platform specific sources
if(SFML_OS_WINDOWS)
    set(PLATFORM_SRC
        ${SRCROOT}/Win32/SleepImpl.cpp
        ${SRCROOT}/Win32/SleepImpl.hpp
    )
    source_group("windows" FILES ${PLATFORM_SRC})
else()
    set(PLATFORM_SRC
        ${SRCROOT}/Unix/SleepImpl.cpp
        ${SRCROOT}/Unix/SleepImpl.hpp
    )

    if(SFML_OS_ANDROID)
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/Android/Activity.hpp
            ${SRCROOT}/Android/Activity.cpp
            ${SRCROOT}/Android/NativeActivity.cpp
            ${SRCROOT}/Android/ResourceStream.cpp
            ${SRCROOT}/Android/ResourceStream.cpp
            ${SRCROOT}/Android/SuspendAwareClock.cpp
        )
    endif()

    source_group("unix" FILES ${PLATFORM_SRC})
endif()

find_package(Threads REQUIRED)

# define the sfml-system target
sfml_add_library(System
                 SOURCES ${SRC} ${PLATFORM_SRC})

# enable precompiled headers
if (SFML_ENABLE_PCH)
    message(VERBOSE "enabling PCH for SFML library 'sfml-system' (reused as the PCH for other SFML libraries)")
    target_precompile_headers(sfml-system PRIVATE ${PROJECT_SOURCE_DIR}/src/SFML/PCH.hpp)
endif()

if(SFML_OS_ANDROID)
    # glad sources
    target_include_directories(sfml-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")
endif()

target_link_libraries(sfml-system PRIVATE Threads::Threads)

if(SFML_OS_LINUX)
    target_link_libraries(sfml-system PRIVATE rt)
elseif(SFML_OS_WINDOWS)
    target_link_libraries(sfml-system PRIVATE winmm)
elseif(SFML_OS_ANDROID)
    target_link_libraries(sfml-system PRIVATE android log)
endif()

if(SFML_ENABLE_STACK_TRACES)
    target_link_libraries(sfml-system PUBLIC cpptrace::cpptrace)
endif()
