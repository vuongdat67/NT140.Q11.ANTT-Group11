#ifndef FILEVAULT_ARCHIVE_FORMAT_HPP
#define FILEVAULT_ARCHIVE_FORMAT_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <span>
#include <filesystem>

namespace filevault::archive {

/**
 * @brief Archive file entry metadata
 */
struct FileEntry {
    std::string filename;
    uint64_t file_size;
    uint64_t offset;           // Offset in data section
    uint64_t modified_time;    // Unix timestamp
    uint32_t permissions;      // File permissions
    
    std::vector<uint8_t> serialize() const;
    static FileEntry deserialize(std::span<const uint8_t> data, size_t& offset);
};

/**
 * @brief Simple archive format handler
 * 
 * Format:
 * [Magic: "FVARCH"] [Version: 1 byte] [Entry count: 4 bytes]
 * [Entry1 metadata] [Entry2 metadata] ...
 * [File1 data] [File2 data] ...
 */
class ArchiveFormat {
public:
    static constexpr char MAGIC[7] = "FVARCH";
    static constexpr uint8_t VERSION = 1;
    
    /**
     * @brief Create archive from multiple files
     */
    static std::vector<uint8_t> create_archive(
        const std::vector<std::filesystem::path>& files
    );
    
    /**
     * @brief Extract files from archive
     */
    static bool extract_archive(
        std::span<const uint8_t> archive_data,
        const std::filesystem::path& output_dir
    );
    
    /**
     * @brief List files in archive
     */
    static std::vector<FileEntry> list_files(
        std::span<const uint8_t> archive_data
    );
    
private:
    static void write_uint32(std::vector<uint8_t>& buffer, uint32_t value);
    static void write_uint64(std::vector<uint8_t>& buffer, uint64_t value);
    static uint32_t read_uint32(std::span<const uint8_t> data, size_t& offset);
    static uint64_t read_uint64(std::span<const uint8_t> data, size_t& offset);
    static std::string read_string(std::span<const uint8_t> data, size_t& offset);
};

} // namespace filevault::archive

#endif // FILEVAULT_ARCHIVE_FORMAT_HPP
