#include <SFML/Base/Assert.hpp>

#include <LoadIntoMemoryUtil.hpp>

#include <fstream>
#include <string>

std::vector<std::byte> loadIntoMemory(const std::string_view& path)
{
    std::ifstream file(std::string(path), std::ios::binary | std::ios::ate);
    SFML_BASE_ASSERT(file);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<std::byte>       buffer(static_cast<std::size_t>(size));
    [[maybe_unused]] const auto& result = file.read(reinterpret_cast<char*>(buffer.data()),
                                                    static_cast<std::streamsize>(size));
    SFML_BASE_ASSERT(result);
    return buffer;
}
