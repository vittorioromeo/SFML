#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/EventUtils.hpp>
#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/Sleep.hpp>

#include <SFML/Base/Optional.hpp>

#include <cstdlib>

// Do we want to showcase direct JNI/NDK interaction?
// Undefine this to get real cross-platform code.
// Uncomment this to try JNI access; this seems to be broken in latest NDKs
//#define USE_JNI

#if defined(USE_JNI)
// These headers are only needed for direct NDK/JDK interaction
#include <android/native_activity.h>
#include <jni.h>

// Since we want to get the native activity from SFML, we'll have to use an
// extra header here:
#include <SFML/System/NativeActivity.hpp>

// NDK/JNI sub example - call Java code from native code
int vibrate(sf::Time duration)
{
    // First we'll need the native activity handle
    ANativeActivity* activity = sf::getNativeActivity();

    // Retrieve the JVM and JNI environment
    JavaVM* vm  = activity->vm;
    JNIEnv* env = activity->env;

    // First, attach this thread to the main thread
    JavaVMAttachArgs attachargs;
    attachargs.version = JNI_VERSION_1_6;
    attachargs.name    = "NativeThread";
    attachargs.group   = nullptr;
    jint res           = vm->AttachCurrentThread(&env, &attachargs);

    if (res == JNI_ERR)
        return EXIT_FAILURE;

    // Retrieve class information
    jclass natact  = env->FindClass("android/app/NativeActivity");
    jclass context = env->FindClass("android/content/Context");

    // Get the value of a constant
    jfieldID fid    = env->GetStaticFieldID(context, "VIBRATOR_SERVICE", "Ljava/lang/String;");
    jobject  svcstr = env->GetStaticObjectField(context, fid);

    // Get the method 'getSystemService' and call it
    jmethodID getss   = env->GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject   vib_obj = env->CallObjectMethod(activity->clazz, getss, svcstr);

    // Get the object's class and retrieve the member name
    jclass    vib_cls = env->GetObjectClass(vib_obj);
    jmethodID vibrate = env->GetMethodID(vib_cls, "vibrate", "(J)V");

    // Determine the timeframe
    jlong length = duration.asMilliseconds();

    // Bzzz!
    env->CallVoidMethod(vib_obj, vibrate, length);

    // Free references
    env->DeleteLocalRef(vib_obj);
    env->DeleteLocalRef(vib_cls);
    env->DeleteLocalRef(svcstr);
    env->DeleteLocalRef(context);
    env->DeleteLocalRef(natact);

    // Detach thread again
    vm->DetachCurrentThread();
}
#endif

// This is the actual Android example. You don't have to write any platform
// specific code, unless you want to use things not directly exposed.
// ('vibrate()' in this example; undefine 'USE_JNI' above to disable it)
int main(int, char**)
{
    // Create the graphics context
    sf::GraphicsContext graphicsContext;

    sf::VideoMode screen(sf::VideoMode::getDesktopMode());

    sf::RenderWindow window(graphicsContext, screen, "");
    window.setFramerateLimit(30);

    const auto texture = sf::Texture::loadFromFile(graphicsContext, "image.png").value();

    sf::Sprite image(texture.getRect());
    image.setPosition(screen.size.to<sf::Vector2f>() / 2.f);
    image.setOrigin(texture.getSize().to<sf::Vector2f>() / 2.f);

    const auto font = sf::Font::openFromFile(graphicsContext, "tuffy.ttf").value();

    sf::Text text(font, "Tap anywhere to move the logo.", 64);
    text.setFillColor(sf::Color::Black);
    text.setPosition({10, 10});

    sf::View view = window.getDefaultView();

    sf::Color background = sf::Color::White;

    // We shouldn't try drawing to the screen while in background
    // so we'll have to track that. You can do minor background
    // work, but keep battery life in mind.
    bool active = true;

    while (true)
    {
        while (const sf::base::Optional event = active ? window.pollEvent() : window.waitEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;

            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                const auto size = resized->size.to<sf::Vector2f>();
                view.setSize(size);
                view.setCenter(size / 2.f);
                window.setView(view);
            }
            else if (event->is<sf::Event::FocusLost>())
            {
                background = sf::Color::Black;
            }
            else if (event->is<sf::Event::FocusGained>())
            {
                background = sf::Color::White;
            }
            // On Android MouseLeft/MouseEntered are (for now) triggered,
            // whenever the app loses or gains focus.
            else if (event->is<sf::Event::MouseLeft>())
            {
                active = false;
            }
            else if (event->is<sf::Event::MouseEntered>())
            {
                active = true;
            }
            else if (const auto* touchBegan = event->getIf<sf::Event::TouchBegan>())
            {
                if (touchBegan->finger == 0)
                {
                    image.setPosition(touchBegan->position.to<sf::Vector2f>());
#if defined(USE_JNI)
                    vibrate(sf::milliseconds(10));
#endif
                }
            }
        }

        if (active)
        {
            window.clear(background);
            window.draw(image, texture);
            window.draw(text);
            window.display();
        }
        else
        {
            sf::sleep(sf::milliseconds(100));
        }
    }
}
