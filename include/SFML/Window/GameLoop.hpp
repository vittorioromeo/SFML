#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Traits/IsRvalueReference.hpp>
#include <SFML/Base/Traits/RemoveCVRef.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class GameLoop
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] ControlFlow
    {
        Continue,
        Break
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ControlFlow continueLoop();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ControlFlow breakLoop();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    static void runImpl(ControlFlow (*func)());

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename F>
    static void run(F&& func)
    {
        if constexpr (base::isRvalueReference<F>)
        {
            thread_local SFML_BASE_REMOVE_CVREF(F) pinnedFunc;
            pinnedFunc = SFML_BASE_MOVE(func);
            runImpl([]() -> ControlFlow { return pinnedFunc(); });
        }
        else
        {
            thread_local F* pinnedFunc;
            pinnedFunc = &func;
            runImpl([]() -> ControlFlow { return (*pinnedFunc)(); });
        }
    }


    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct Runner
    {
        template <typename F>
        void operator|(F&& func)
        {
            run(SFML_BASE_FORWARD(func));
        }
    };
};

} // namespace sf::priv


////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SFML_GAME_LOOP ::sf::priv::GameLoop::Runner{} | [&]

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
#define SFML_GAME_LOOP_BREAK return ::sf::priv::GameLoop::breakLoop()

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
#define SFML_GAME_LOOP_CONTINUE return ::sf::priv::GameLoop::continueLoop()


////////////////////////////////////////////////////////////
/// \namespace sf::GameLoop
/// \ingroup window
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
