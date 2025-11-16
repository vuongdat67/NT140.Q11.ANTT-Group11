#include "filevault/utils/file_io.hpp"
#include <fstream>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace filevault {
namespace utils {

core::Result<std::vector<uint8_t>> FileIO::read_file(const std::string& path) {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return core::Result<std::vector<uint8_t>>::error("Cannot open file: " + path);
        }
        
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        
        if (!file) {
            return core::Result<std::vector<uint8_t>>::error("Failed to read file: " + path);
        }
        
        spdlog::debug("Read {} bytes from {}", size, path);
        return core::Result<std::vector<uint8_t>>::ok(std::move(data));
        
    } catch (const std::exception& e) {
        return core::Result<std::vector<uint8_t>>::error(std::string("Exception: ") + e.what());
    }
}

core::Result<void> FileIO::write_file(const std::string& path, std::span<const uint8_t> data) {
    try {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            return core::Result<void>::error("Cannot create file: " + path);
        }
        
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        
        if (!file) {
            return core::Result<void>::error("Failed to write file: " + path);
        }
        
        spdlog::debug("Wrote {} bytes to {}", data.size(), path);
        return core::Result<void>::ok();
        
    } catch (const std::exception& e) {
        return core::Result<void>::error(std::string("Exception: ") + e.what());
    }
}

bool FileIO::file_exists(const std::string& path) {
    return std::filesystem::exists(path);
}

size_t FileIO::file_size(const std::string& path) {
    if (!file_exists(path)) {
        return 0;
    }
    return std::filesystem::file_size(path);
}

} // namespace utils
} // namespace filevault
