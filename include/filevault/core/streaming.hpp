#ifndef FILEVAULT_CORE_STREAMING_HPP
#define FILEVAULT_CORE_STREAMING_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include "types.hpp"
#include "result.hpp"

namespace filevault {
namespace core {

/**
 * @brief Chunk information for streaming encryption
 */
struct ChunkInfo {
    size_t chunk_index;
    size_t chunk_size;
    size_t total_chunks;
    size_t bytes_processed;
    size_t total_bytes;
};

/**
 * @brief Progress callback for streaming operations
 * @param info Current chunk information
 * @return false to cancel operation, true to continue
 */
using StreamProgressCallback = std::function<bool(const ChunkInfo& info)>;

/**
 * @brief Configuration for streaming encryption
 */
struct StreamingConfig {
    size_t chunk_size = 64 * 1024 * 1024;  // 64MB default chunk size
    AlgorithmType algorithm = AlgorithmType::AES_256_GCM;
    KDFType kdf = KDFType::ARGON2ID;
    SecurityLevel level = SecurityLevel::STRONG;
    CompressionType compression = CompressionType::NONE;
    int compression_level = 6;
    StreamProgressCallback progress_callback = nullptr;
};

/**
 * @brief Result of streaming operation
 */
struct StreamingResult {
    bool success = false;
    std::string error_message;
    size_t bytes_processed = 0;
    size_t chunks_processed = 0;
    double processing_time_ms = 0.0;
    double throughput_mbps = 0.0;
};

/**
 * @brief Streaming encryption/decryption for large files
 * 
 * Uses chunk-based processing to handle files larger than available RAM.
 * Each chunk is encrypted independently with a unique nonce derived from
 * the base nonce and chunk index.
 * 
 * File format for streaming:
 * [Header][Chunk1][Chunk2]...[ChunkN][Footer]
 * 
 * Each chunk:
 * [4 bytes: chunk_size][12 bytes: nonce][encrypted_data][16 bytes: tag]
 */
class StreamingCrypto {
public:
    /**
     * @brief Encrypt a large file using streaming
     * @param input_path Path to input file
     * @param output_path Path to output file
     * @param password Encryption password
     * @param config Streaming configuration
     * @return Result of the operation
     */
    static StreamingResult encrypt_file(
        const std::string& input_path,
        const std::string& output_path,
        const std::string& password,
        const StreamingConfig& config = {}
    );
    
    /**
     * @brief Decrypt a large file using streaming
     * @param input_path Path to encrypted file
     * @param output_path Path to output file
     * @param password Decryption password
     * @param progress_callback Optional progress callback
     * @return Result of the operation
     */
    static StreamingResult decrypt_file(
        const std::string& input_path,
        const std::string& output_path,
        const std::string& password,
        StreamProgressCallback progress_callback = nullptr
    );
    
    /**
     * @brief Check if a file should use streaming (based on size)
     * @param file_path Path to file
     * @param threshold Size threshold (default 100MB)
     * @return true if file is larger than threshold
     */
    static bool should_use_streaming(
        const std::string& file_path,
        size_t threshold = 100 * 1024 * 1024
    );
    
    /**
     * @brief Get recommended chunk size based on available memory
     * @return Recommended chunk size in bytes
     */
    static size_t get_recommended_chunk_size();

private:
    /**
     * @brief Derive chunk-specific nonce from base nonce and chunk index
     */
    static std::vector<uint8_t> derive_chunk_nonce(
        const std::vector<uint8_t>& base_nonce,
        size_t chunk_index
    );
    
    /**
     * @brief Write streaming file header
     */
    static bool write_stream_header(
        std::ofstream& file,
        const StreamingConfig& config,
        const std::vector<uint8_t>& salt,
        const std::vector<uint8_t>& base_nonce,
        size_t total_size,
        size_t chunk_count
    );
    
    /**
     * @brief Read streaming file header
     */
    static bool read_stream_header(
        std::ifstream& file,
        StreamingConfig& config,
        std::vector<uint8_t>& salt,
        std::vector<uint8_t>& base_nonce,
        size_t& original_size,
        size_t& chunk_count
    );
};

} // namespace core
} // namespace filevault

#endif // FILEVAULT_CORE_STREAMING_HPP
