#include <LoadIntoMemory.hpp>
#include <fstream>

#include <cassert>

std::vector<std::byte> loadIntoMemory(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    assert(file);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<std::byte>       buffer(static_cast<std::size_t>(size));
    [[maybe_unused]] const auto& result = file.read(reinterpret_cast<char*>(buffer.data()),
                                                    static_cast<std::streamsize>(size));
    assert(result);
    return buffer;
}
