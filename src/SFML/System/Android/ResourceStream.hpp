#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/System/InputStream.hpp>

#include <SFML/Base/UniquePtr.hpp>

#include <android/asset_manager.h>

#include <string>


namespace sf
{
class Path;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Read from Android asset files
///
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API ResourceStream : public InputStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// \param filename Filename of the asset
    ///
    ////////////////////////////////////////////////////////////
    ResourceStream(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Read data from the asset
    ///
    /// \param data Buffer where the asset data is copied
    /// \param size Number of bytes read
    ///
    /// \return The number of bytes actually read, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    base::Optional<std::size_t> read(void* data, std::size_t size) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current reading position in the asset file
    ///
    /// \param position The position to seek to, from the beginning
    ///
    /// \return The position actually sought to, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    base::Optional<std::size_t> seek(std::size_t position) override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current reading position in the asset file
    ///
    /// \return The current position, or `base::nullOpt` on error.
    ///
    ////////////////////////////////////////////////////////////
    base::Optional<std::size_t> tell() override;

    ////////////////////////////////////////////////////////////
    /// \brief Return the size of the asset file
    ///
    /// \return The total number of bytes available in the asset, or `base::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    base::Optional<std::size_t> getSize() override;

private:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    struct AAssetDeleter
    {
        void operator()(AAsset*);
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    UniquePtr<AAsset, AAssetDeleter> m_file; ///< The asset file to read
};

} // namespace sf::priv
