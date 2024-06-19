// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <cassert>

#if __has_builtin(__is_base_of)
namespace sf::priv
{

template <typename B, typename D>
struct IsBaseOf
{
    enum
    {
        value = __is_base_of(B, D)
    };
};

} // namespace sf::priv

#else
#include <type_traits>

namespace sf::priv
{
template <typename B, typename D>
using IsBaseOf = std::is_base_of<B, D>;
}
#endif

namespace sf::priv
{

// clang-format off
template<bool, typename = void> struct EnableIfImpl          { };
template<typename T>            struct EnableIfImpl<true, T> { using type = T; };
// clang-format on

template <bool B, typename T = void>
using EnableIf = typename EnableIfImpl<B, T>::type;

struct UniquePtrDefaultDeleter
{
    template <typename T>
    [[gnu::always_inline]] void operator()(T* const ptr) const noexcept
    {
        delete ptr;
    }
};

template <typename T, typename TDeleter = UniquePtrDefaultDeleter>
class UniquePtr : private TDeleter
{
    template <typename, typename>
    friend class UniquePtr;

private:
    T* _ptr;

public:
    [[nodiscard, gnu::always_inline]] explicit UniquePtr() noexcept : _ptr{nullptr}
    {
    }

    [[nodiscard, gnu::always_inline]] /* implicit */ UniquePtr(decltype(nullptr)) noexcept : _ptr{nullptr}
    {
    }

    [[nodiscard, gnu::always_inline]] explicit UniquePtr(T* ptr) noexcept : _ptr{ptr}
    {
    }

    [[nodiscard, gnu::always_inline]] explicit UniquePtr(T* ptr, const TDeleter& deleter) noexcept :
    TDeleter{deleter},
    _ptr{ptr}
    {
    }

    [[gnu::always_inline]] ~UniquePtr() noexcept
    {
        if (_ptr != nullptr)
        {
            static_cast<TDeleter*>(this)->operator()(_ptr);
        }
    }

    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    template <typename U, typename UDeleter, typename = EnableIf<IsBaseOf<T, U>::value>>
    [[nodiscard, gnu::always_inline]] UniquePtr(UniquePtr<U, UDeleter>&& rhs) noexcept : TDeleter{static_cast<UDeleter&&>(rhs)}, _ptr{rhs._ptr}
    {
        rhs._ptr = nullptr;
    }

    template <typename U, typename UDeleter, typename = EnableIf<IsBaseOf<T, U>::value>>
    [[gnu::always_inline]] UniquePtr& operator=(UniquePtr<U, UDeleter>&& rhs) noexcept
    {
        (*static_cast<TDeleter*>(this)) = static_cast<UDeleter&&>(rhs);

        reset(rhs._ptr);
        rhs._ptr = nullptr;

        return *this;
    }

    [[nodiscard, gnu::always_inline]] T* get() const noexcept
    {
        return _ptr;
    }

    [[nodiscard, gnu::always_inline]] T& operator*() const noexcept
    {
        assert(_ptr != nullptr);
        return *_ptr;
    }

    [[nodiscard, gnu::always_inline]] T* operator->() const noexcept
    {
        assert(_ptr != nullptr);
        return _ptr;
    }

    [[nodiscard, gnu::always_inline]] explicit operator bool() const noexcept
    {
        return _ptr != nullptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator==(const T* ptr) const noexcept
    {
        return _ptr == ptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator!=(const T* ptr) const noexcept
    {
        return _ptr != ptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator==(decltype(nullptr)) const noexcept
    {
        return _ptr == nullptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator!=(decltype(nullptr)) const noexcept
    {
        return _ptr != nullptr;
    }

    [[gnu::always_inline]] void reset(T* const ptr = nullptr) noexcept
    {
        if (_ptr != nullptr)
        {
            static_cast<TDeleter*>(this)->operator()(_ptr);
        }

        _ptr = ptr;
    }
};

template <typename T, typename... Ts>
[[nodiscard, gnu::always_inline]] inline UniquePtr<T> makeUnique(Ts&&... xs)
{
    return UniquePtr<T>{new T{static_cast<Ts&&>(xs)...}};
}

} // namespace sf::priv
