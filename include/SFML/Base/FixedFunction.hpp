////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Macros.hpp>
#include <SFML/Base/MaxAlignT.hpp>
#include <SFML/Base/PlacementNew.hpp>
#include <SFML/Base/SizeT.hpp>
#include <SFML/Base/Traits/IsRvalueReference.hpp>
#include <SFML/Base/Traits/RemoveReference.hpp>


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Non-allocating `std::function` alternatifve with fixed storage size
///
////////////////////////////////////////////////////////////
template <typename TSignature, SizeT TStorageSize>
class FixedFunction;

////////////////////////////////////////////////////////////
/// \brief Non-allocating `std::function` alternatifve with fixed storage size
///
////////////////////////////////////////////////////////////
template <typename TReturn, typename... Ts, SizeT TStorageSize>
class FixedFunction<TReturn(Ts...), TStorageSize>
{
private:
    enum Operation : unsigned char
    {
        Destroy       = 0u,
        MoveConstruct = 1u,
        CopyConstruct = 2u,
    };

    using RetType = TReturn;

    using FnPtrType  = RetType (*)(Ts...);
    using MethodType = RetType (*)(char*, FnPtrType, Ts...);
    using AllocType  = void (*)(char*, void* objectPtr, Operation operation);

    union
    {
        alignas(MaxAlignT) char objStorage[TStorageSize];
        FnPtrType functionPtr;
    };

    MethodType m_methodPtr;
    AllocType  m_allocPtr;

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] FixedFunction() noexcept : functionPtr{nullptr}, m_methodPtr{nullptr}, m_allocPtr{nullptr}
    {
    }


    ////////////////////////////////////////////////////////////
    /**
     * @brief FixedFunction Constructor from functional object.
     * @param f Functor object will be stored in the internal objStorage
     * using move constructor. Unmovable objects are prohibited explicitly.
     */
    template <typename TFFwd>
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    [[nodiscard]] FixedFunction(TFFwd&& f) noexcept : FixedFunction()
    {
        using UnrefType = SFML_BASE_REMOVE_REFERENCE(TFFwd);
        static_assert(sizeof(UnrefType) < TStorageSize);

        // NOLINTNEXTLINE(readability-non-const-parameter)
        m_methodPtr = [](char* s, FnPtrType, Ts... xs) -> RetType
        { return reinterpret_cast<UnrefType*>(s)->operator()(xs...); };

        // NOLINTNEXTLINE(readability-non-const-parameter)
        m_allocPtr = [](char* s, void* o, Operation operation)
        {
            if (operation == Operation::Destroy)
            {
                SFML_BASE_ASSERT(s != nullptr);
                reinterpret_cast<UnrefType*>(s)->~UnrefType();
            }
            else if (operation == Operation::MoveConstruct)
            {
                SFML_BASE_ASSERT(o != nullptr);
                SFML_BASE_PLACEMENT_NEW(s) UnrefType(SFML_BASE_MOVE(*static_cast<UnrefType*>(o)));
            }
            else
            {
                SFML_BASE_ASSERT(operation == Operation::CopyConstruct);

                SFML_BASE_ASSERT(o != nullptr);
                SFML_BASE_PLACEMENT_NEW(s) UnrefType(*static_cast<const UnrefType*>(o));
            }
        };

        m_allocPtr(objStorage, &f, isRvalueReference<TFFwd&&> ? Operation::MoveConstruct : Operation::CopyConstruct);
    }


    ////////////////////////////////////////////////////////////
    template <typename TFReturn, typename... TFs>
    FixedFunction(TFReturn (*f)(TFs...)) noexcept :
    functionPtr{f},
    m_methodPtr{[](bool, const char*, FnPtrType xf, Ts... xs) { return static_cast<decltype(f)>(xf)(xs...); }},
    m_allocPtr{nullptr}
    {
    }


    ////////////////////////////////////////////////////////////
    FixedFunction(const FixedFunction& rhs) : FixedFunction()
    {
        m_methodPtr = rhs.m_methodPtr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr = rhs.functionPtr;
            return;
        }

        m_allocPtr = rhs.m_allocPtr;

        SFML_BASE_ASSERT(m_allocPtr != nullptr);
        m_allocPtr(objStorage, const_cast<char*>(rhs.objStorage), Operation::CopyConstruct);
    }


    ////////////////////////////////////////////////////////////
    FixedFunction& operator=(const FixedFunction& rhs)
    {
        m_methodPtr = rhs.m_methodPtr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr = rhs.functionPtr;
            return *this;
        }

        m_allocPtr = rhs.m_allocPtr;

        SFML_BASE_ASSERT(m_allocPtr != nullptr);
        m_allocPtr(objStorage, const_cast<char*>(rhs.objStorage), Operation::CopyConstruct);

        return *this;
    }


    ////////////////////////////////////////////////////////////
    FixedFunction(FixedFunction&& rhs) noexcept : FixedFunction()
    {
        m_methodPtr = rhs.m_methodPtr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr = rhs.functionPtr;
            return;
        }

        m_allocPtr = rhs.m_allocPtr;

        SFML_BASE_ASSERT(m_allocPtr != nullptr);
        m_allocPtr(objStorage, rhs.objStorage, Operation::MoveConstruct);
    }


    ////////////////////////////////////////////////////////////
    FixedFunction& operator=(FixedFunction&& rhs) noexcept
    {
        m_methodPtr = rhs.m_methodPtr;

        if (rhs.m_allocPtr == nullptr)
        {
            functionPtr = rhs.functionPtr;
            return *this;
        }

        m_allocPtr = rhs.m_allocPtr;

        SFML_BASE_ASSERT(m_allocPtr != nullptr);
        m_allocPtr(objStorage, rhs.objStorage, Operation::MoveConstruct);

        return *this;
    }


    ////////////////////////////////////////////////////////////
    ~FixedFunction() noexcept
    {
        if (m_allocPtr)
            m_allocPtr(objStorage, nullptr, Operation::Destroy);
    }


    ////////////////////////////////////////////////////////////
    template <typename... TArgs>
    RetType operator()(TArgs&&... args)
    {
        SFML_BASE_ASSERT(m_methodPtr != nullptr);
        return m_methodPtr(objStorage, functionPtr, SFML_BASE_FORWARD(args)...);
    }


    ////////////////////////////////////////////////////////////
    template <typename... TArgs>
    RetType operator()(TArgs&&... args) const
    {
        SFML_BASE_ASSERT(m_methodPtr != nullptr);
        return m_methodPtr(const_cast<char*>(objStorage), functionPtr, SFML_BASE_FORWARD(args)...);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit operator bool() const
    {
        return m_methodPtr != nullptr;
    }
};

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \class sf::base::FixedFunction
/// \ingroup system
///
/// Non-allocating `std::function` alternatifve with fixed storage size
///
////////////////////////////////////////////////////////////
