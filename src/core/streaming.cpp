/**
 * @file streaming.cpp
 * @brief Streaming encryption implementation for large files
 */

#include "filevault/core/streaming.hpp"
#include "filevault/core/crypto_engine.hpp"
#include "filevault/compression/compressor.hpp"
#include <botan/auto_rng.h>
#include <spdlog/spdlog.h>
#include <chrono>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

namespace filevault {
namespace core {

// Magic bytes for streaming format: "FVST" (FileVault STreaming)
static constexpr uint8_t STREAM_MAGIC[4] = {'F', 'V', 'S', 'T'};
static constexpr uint8_t STREAM_VERSION = 1;

size_t StreamingCrypto::get_recommended_chunk_size() {
    size_t available_memory = 0;
    
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        available_memory = static_cast<size_t>(status.ullAvailPhys);
    }
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        available_memory = info.freeram * info.mem_unit;
    }
#endif
    
    // Use 1/4 of available memory, capped between 16MB and 256MB
    size_t chunk_size = available_memory / 4;
    chunk_size = (std::max)(chunk_size, size_t(16 * 1024 * 1024));   // Min 16MB
    chunk_size = (std::min)(chunk_size, size_t(256 * 1024 * 1024)); // Max 256MB
    
    // Round down to nearest MB
    chunk_size = (chunk_size / (1024 * 1024)) * (1024 * 1024);
    
    return chunk_size;
}

bool StreamingCrypto::should_use_streaming(const std::string& file_path, size_t threshold) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file) return false;
    
    auto size = file.tellg();
    return static_cast<size_t>(size) > threshold;
}

std::vector<uint8_t> StreamingCrypto::derive_chunk_nonce(
    const std::vector<uint8_t>& base_nonce,
    size_t chunk_index
) {
    // XOR chunk index into last 4 bytes of nonce
    std::vector<uint8_t> chunk_nonce = base_nonce;
    
    // Ensure nonce is at least 12 bytes
    if (chunk_nonce.size() < 12) {
        chunk_nonce.resize(12, 0);
    }
    
    // XOR chunk index into bytes 8-11
    for (int i = 0; i < 4; ++i) {
        chunk_nonce[8 + i] ^= static_cast<uint8_t>((chunk_index >> (i * 8)) & 0xFF);
    }
    
    return chunk_nonce;
}

bool StreamingCrypto::write_stream_header(
    std::ofstream& file,
    const StreamingConfig& config,
    const std::vector<uint8_t>& salt,
    const std::vector<uint8_t>& base_nonce,
    size_t total_size,
    size_t chunk_count
) {
    // Write magic bytes
    file.write(reinterpret_cast<const char*>(STREAM_MAGIC), 4);
    
    // Write version
    file.write(reinterpret_cast<const char*>(&STREAM_VERSION), 1);
    
    // Write algorithm type (1 byte)
    uint8_t algo = static_cast<uint8_t>(config.algorithm);
    file.write(reinterpret_cast<const char*>(&algo), 1);
    
    // Write KDF type (1 byte)
    uint8_t kdf = static_cast<uint8_t>(config.kdf);
    file.write(reinterpret_cast<const char*>(&kdf), 1);
    
    // Write compression type (1 byte)
    uint8_t comp = static_cast<uint8_t>(config.compression);
    file.write(reinterpret_cast<const char*>(&comp), 1);
    
    // Write chunk size (8 bytes)
    uint64_t chunk_sz = config.chunk_size;
    file.write(reinterpret_cast<const char*>(&chunk_sz), 8);
    
    // Write total size (8 bytes)
    uint64_t total_sz = total_size;
    file.write(reinterpret_cast<const char*>(&total_sz), 8);
    
    // Write chunk count (4 bytes)
    uint32_t chunks = static_cast<uint32_t>(chunk_count);
    file.write(reinterpret_cast<const char*>(&chunks), 4);
    
    // Write salt length and salt (1 + N bytes)
    uint8_t salt_len = static_cast<uint8_t>(salt.size());
    file.write(reinterpret_cast<const char*>(&salt_len), 1);
    file.write(reinterpret_cast<const char*>(salt.data()), salt.size());
    
    // Write nonce length and base nonce (1 + N bytes)
    uint8_t nonce_len = static_cast<uint8_t>(base_nonce.size());
    file.write(reinterpret_cast<const char*>(&nonce_len), 1);
    file.write(reinterpret_cast<const char*>(base_nonce.data()), base_nonce.size());
    
    return file.good();
}

bool StreamingCrypto::read_stream_header(
    std::ifstream& file,
    StreamingConfig& config,
    std::vector<uint8_t>& salt,
    std::vector<uint8_t>& base_nonce,
    size_t& original_size,
    size_t& chunk_count
) {
    // Read and verify magic bytes
    uint8_t magic[4];
    file.read(reinterpret_cast<char*>(magic), 4);
    if (std::memcmp(magic, STREAM_MAGIC, 4) != 0) {
        spdlog::error("Invalid streaming file format");
        return false;
    }
    
    // Read version
    uint8_t version;
    file.read(reinterpret_cast<char*>(&version), 1);
    if (version != STREAM_VERSION) {
        spdlog::error("Unsupported streaming format version: {}", version);
        return false;
    }
    
    // Read algorithm type
    uint8_t algo;
    file.read(reinterpret_cast<char*>(&algo), 1);
    config.algorithm = static_cast<AlgorithmType>(algo);
    
    // Read KDF type
    uint8_t kdf;
    file.read(reinterpret_cast<char*>(&kdf), 1);
    config.kdf = static_cast<KDFType>(kdf);
    
    // Read compression type
    uint8_t comp;
    file.read(reinterpret_cast<char*>(&comp), 1);
    config.compression = static_cast<CompressionType>(comp);
    
    // Read chunk size
    uint64_t chunk_sz;
    file.read(reinterpret_cast<char*>(&chunk_sz), 8);
    config.chunk_size = static_cast<size_t>(chunk_sz);
    
    // Read total size
    uint64_t total_sz;
    file.read(reinterpret_cast<char*>(&total_sz), 8);
    original_size = static_cast<size_t>(total_sz);
    
    // Read chunk count
    uint32_t chunks;
    file.read(reinterpret_cast<char*>(&chunks), 4);
    chunk_count = chunks;
    
    // Read salt
    uint8_t salt_len;
    file.read(reinterpret_cast<char*>(&salt_len), 1);
    salt.resize(salt_len);
    file.read(reinterpret_cast<char*>(salt.data()), salt_len);
    
    // Read base nonce
    uint8_t nonce_len;
    file.read(reinterpret_cast<char*>(&nonce_len), 1);
    base_nonce.resize(nonce_len);
    file.read(reinterpret_cast<char*>(base_nonce.data()), nonce_len);
    
    return file.good();
}

StreamingResult StreamingCrypto::encrypt_file(
    const std::string& input_path,
    const std::string& output_path,
    const std::string& password,
    const StreamingConfig& config
) {
    StreamingResult result;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Open input file
        std::ifstream input(input_path, std::ios::binary | std::ios::ate);
        if (!input) {
            result.error_message = "Failed to open input file: " + input_path;
            return result;
        }
        
        size_t file_size = input.tellg();
        input.seekg(0);
        
        spdlog::info("Streaming encryption: {} ({} bytes)", input_path, file_size);
        
        // Calculate chunk count
        size_t chunk_size = config.chunk_size;
        size_t chunk_count = (file_size + chunk_size - 1) / chunk_size;
        
        // Initialize crypto engine
        CryptoEngine engine;
        engine.initialize();
        
        // Generate salt and derive key
        auto salt = CryptoEngine::generate_salt(32);
        auto base_nonce = CryptoEngine::generate_nonce(12);
        
        EncryptionConfig enc_config;
        enc_config.algorithm = config.algorithm;
        enc_config.kdf = config.kdf;
        enc_config.level = config.level;
        enc_config.apply_security_level();
        
        auto key = engine.derive_key(password, salt, enc_config);
        
        // Get algorithm
        auto* algo = engine.get_algorithm(config.algorithm);
        if (!algo) {
            result.error_message = "Algorithm not available";
            return result;
        }
        
        // Open output file
        std::ofstream output(output_path, std::ios::binary);
        if (!output) {
            result.error_message = "Failed to create output file: " + output_path;
            return result;
        }
        
        // Write header
        if (!write_stream_header(output, config, salt, base_nonce, file_size, chunk_count)) {
            result.error_message = "Failed to write stream header";
            return result;
        }
        
        // Process chunks
        std::vector<uint8_t> chunk_buffer(chunk_size);
        size_t bytes_processed = 0;
        
        for (size_t i = 0; i < chunk_count; ++i) {
            // Read chunk
            size_t bytes_to_read = (std::min)(chunk_size, file_size - bytes_processed);
            input.read(reinterpret_cast<char*>(chunk_buffer.data()), bytes_to_read);
            
            if (!input && !input.eof()) {
                result.error_message = "Failed to read input chunk";
                return result;
            }
            
            // Compress if enabled
            std::vector<uint8_t> data_to_encrypt(chunk_buffer.begin(), 
                                                  chunk_buffer.begin() + bytes_to_read);
            
            if (config.compression != CompressionType::NONE) {
                auto compressor = compression::CompressionService::create(config.compression);
                if (compressor) {
                    auto comp_result = compressor->compress(data_to_encrypt, config.compression_level);
                    if (comp_result.success && comp_result.data.size() < data_to_encrypt.size()) {
                        data_to_encrypt = std::move(comp_result.data);
                    }
                }
            }
            
            // Generate chunk-specific nonce
            auto chunk_nonce = derive_chunk_nonce(base_nonce, i);
            enc_config.nonce = chunk_nonce;
            
            // Encrypt chunk
            auto enc_result = algo->encrypt(data_to_encrypt, key, enc_config);
            if (!enc_result.success) {
                result.error_message = "Encryption failed at chunk " + std::to_string(i);
                return result;
            }
            
            // Write encrypted chunk: [4 bytes size][data][16 bytes tag]
            uint32_t enc_size = static_cast<uint32_t>(enc_result.data.size());
            output.write(reinterpret_cast<const char*>(&enc_size), 4);
            output.write(reinterpret_cast<const char*>(enc_result.data.data()), enc_result.data.size());
            
            // Write tag if present
            if (enc_result.tag.has_value()) {
                output.write(reinterpret_cast<const char*>(enc_result.tag.value().data()), 
                            enc_result.tag.value().size());
            }
            
            bytes_processed += bytes_to_read;
            result.chunks_processed++;
            
            // Progress callback
            if (config.progress_callback) {
                ChunkInfo info{i, bytes_to_read, chunk_count, bytes_processed, file_size};
                if (!config.progress_callback(info)) {
                    result.error_message = "Operation cancelled by user";
                    return result;
                }
            }
        }
        
        result.bytes_processed = bytes_processed;
        result.success = true;
        
    } catch (const std::exception& e) {
        result.error_message = std::string("Streaming encryption failed: ") + e.what();
        return result;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.throughput_mbps = (result.bytes_processed / 1024.0 / 1024.0) / (result.processing_time_ms / 1000.0);
    
    spdlog::info("Streaming encryption completed: {} chunks, {:.2f} MB/s", 
                 result.chunks_processed, result.throughput_mbps);
    
    return result;
}

StreamingResult StreamingCrypto::decrypt_file(
    const std::string& input_path,
    const std::string& output_path,
    const std::string& password,
    StreamProgressCallback progress_callback
) {
    StreamingResult result;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Open input file
        std::ifstream input(input_path, std::ios::binary);
        if (!input) {
            result.error_message = "Failed to open input file: " + input_path;
            return result;
        }
        
        // Read header
        StreamingConfig config;
        std::vector<uint8_t> salt, base_nonce;
        size_t original_size, chunk_count;
        
        if (!read_stream_header(input, config, salt, base_nonce, original_size, chunk_count)) {
            result.error_message = "Failed to read stream header";
            return result;
        }
        
        spdlog::info("Streaming decryption: {} chunks, {} bytes original", chunk_count, original_size);
        
        // Initialize crypto engine
        CryptoEngine engine;
        engine.initialize();
        
        // Derive key
        EncryptionConfig enc_config;
        enc_config.algorithm = config.algorithm;
        enc_config.kdf = config.kdf;
        enc_config.level = config.level;
        enc_config.apply_security_level();
        
        auto key = engine.derive_key(password, salt, enc_config);
        
        // Get algorithm
        auto* algo = engine.get_algorithm(config.algorithm);
        if (!algo) {
            result.error_message = "Algorithm not available";
            return result;
        }
        
        // Open output file
        std::ofstream output(output_path, std::ios::binary);
        if (!output) {
            result.error_message = "Failed to create output file: " + output_path;
            return result;
        }
        
        // Process chunks
        size_t bytes_processed = 0;
        
        for (size_t i = 0; i < chunk_count; ++i) {
            // Read encrypted chunk size
            uint32_t enc_size;
            input.read(reinterpret_cast<char*>(&enc_size), 4);
            
            // Read encrypted data
            std::vector<uint8_t> encrypted(enc_size);
            input.read(reinterpret_cast<char*>(encrypted.data()), enc_size);
            
            // Read tag (16 bytes for GCM)
            std::vector<uint8_t> tag(16);
            input.read(reinterpret_cast<char*>(tag.data()), 16);
            
            // Generate chunk-specific nonce
            auto chunk_nonce = derive_chunk_nonce(base_nonce, i);
            enc_config.nonce = chunk_nonce;
            enc_config.tag = tag;
            
            // Decrypt chunk
            auto dec_result = algo->decrypt(encrypted, key, enc_config);
            if (!dec_result.success) {
                result.error_message = "Decryption failed at chunk " + std::to_string(i) + 
                                       ": " + dec_result.error_message;
                return result;
            }
            
            // Decompress if needed
            std::vector<uint8_t> plaintext = std::move(dec_result.data);
            if (config.compression != CompressionType::NONE) {
                auto decompressor = compression::CompressionService::create(config.compression);
                if (decompressor) {
                    auto decomp_result = decompressor->decompress(plaintext);
                    if (decomp_result.success) {
                        plaintext = std::move(decomp_result.data);
                    }
                }
            }
            
            // Write decrypted data
            output.write(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
            
            bytes_processed += plaintext.size();
            result.chunks_processed++;
            
            // Progress callback
            if (progress_callback) {
                ChunkInfo info{i, plaintext.size(), chunk_count, bytes_processed, original_size};
                if (!progress_callback(info)) {
                    result.error_message = "Operation cancelled by user";
                    return result;
                }
            }
        }
        
        result.bytes_processed = bytes_processed;
        result.success = true;
        
    } catch (const std::exception& e) {
        result.error_message = std::string("Streaming decryption failed: ") + e.what();
        return result;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.throughput_mbps = (result.bytes_processed / 1024.0 / 1024.0) / (result.processing_time_ms / 1000.0);
    
    spdlog::info("Streaming decryption completed: {} chunks, {:.2f} MB/s", 
                 result.chunks_processed, result.throughput_mbps);
    
    return result;
}

} // namespace core
} // namespace filevault
