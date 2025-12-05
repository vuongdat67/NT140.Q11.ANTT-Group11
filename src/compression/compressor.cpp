#include "filevault/compression/compressor.hpp"
#include <zlib.h>
#include <libbz3.h>  // BZIP3 API
#include <lzma.h>
#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <fmt/core.h>

namespace filevault {
namespace compression {

// ============================================================================
// CompressionService
// ============================================================================

std::unique_ptr<ICompressor> CompressionService::create(core::CompressionType type) {
    switch (type) {
        case core::CompressionType::ZLIB:
            return std::make_unique<ZlibCompressor>();
        case core::CompressionType::BZIP2:
            return std::make_unique<Bzip2Compressor>();  // Using BZIP3 API
        case core::CompressionType::LZMA:
            return std::make_unique<LzmaCompressor>();
        case core::CompressionType::NONE:
            throw std::invalid_argument("Cannot create compressor for NONE type");
        default:
            throw std::invalid_argument("Unknown compression type");
    }
}

std::string CompressionService::get_algorithm_name(core::CompressionType type) {
    switch (type) {
        case core::CompressionType::NONE: return "none";
        case core::CompressionType::ZLIB: return "zlib";
        case core::CompressionType::BZIP2: return "bzip2";
        case core::CompressionType::LZMA: return "lzma";
        default: return "unknown";
    }
}

core::CompressionType CompressionService::parse_algorithm(const std::string& name) {
    if (name == "none") return core::CompressionType::NONE;
    if (name == "zlib") return core::CompressionType::ZLIB;
    if (name == "bzip2" || name == "bz2") return core::CompressionType::BZIP2;
    if (name == "lzma" || name == "xz") return core::CompressionType::LZMA;
    
    throw std::invalid_argument("Unknown compression algorithm: " + name);
}

// ============================================================================
// ZlibCompressor
// ============================================================================

CompressionResult ZlibCompressor::compress(
    std::span<const uint8_t> input,
    int level
) {
    CompressionResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        // Clamp level to valid range
        level = std::clamp(level, 1, 9);
        
        // Allocate output buffer (worst case: original size + 0.1% + 12 bytes)
        uLongf dest_len = compressBound(input.size());
        result.data.resize(dest_len);
        
        // Compress
        int ret = ::compress2(
            result.data.data(),
            &dest_len,
            input.data(),
            input.size(),
            level
        );
        
        if (ret != Z_OK) {
            result.success = false;
            result.error_message = fmt::format("zlib compression failed: error {}", ret);
            return result;
        }
        
        // Resize to actual size
        result.data.resize(dest_len);
        
        result.success = true;
        result.original_size = input.size();
        result.compressed_size = dest_len;
        result.compression_ratio = 100.0 * (1.0 - static_cast<double>(dest_len) / input.size());
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

CompressionResult ZlibCompressor::decompress(std::span<const uint8_t> input) {
    CompressionResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        // Start with 4x size, grow if needed
        uLongf dest_len = input.size() * 4;
        result.data.resize(dest_len);
        
        int ret = Z_BUF_ERROR;
        int attempts = 0;
        
        // Try decompression, growing buffer if needed
        while (ret == Z_BUF_ERROR && attempts < 10) {
            dest_len = result.data.size();
            ret = ::uncompress(
                result.data.data(),
                &dest_len,
                input.data(),
                input.size()
            );
            
            if (ret == Z_BUF_ERROR) {
                result.data.resize(result.data.size() * 2);
                attempts++;
            }
        }
        
        if (ret != Z_OK) {
            result.success = false;
            result.error_message = fmt::format("zlib decompression failed: error {}", ret);
            return result;
        }
        
        result.data.resize(dest_len);
        
        result.success = true;
        result.original_size = input.size();
        result.compressed_size = dest_len;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

// ============================================================================
// Bzip2Compressor - Using BZIP3 API
// ============================================================================

CompressionResult Bzip2Compressor::compress(
    std::span<const uint8_t> input,
    int level
) {
    CompressionResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (input.empty()) {
            result.success = false;
            result.error_message = "Empty input";
            return result;
        }
        
        // BZIP3 block size (65KB to 511MB, recommended: 1MB to 8MB)
        // Level 1-9 maps to block sizes
        int32_t block_size;
        if (level <= 3) {
            block_size = 1 * 1024 * 1024;      // 1MB - fast
        } else if (level <= 6) {
            block_size = 4 * 1024 * 1024;      // 4MB - balanced
        } else {
            block_size = 8 * 1024 * 1024;      // 8MB - best compression
        }
        
        // Calculate output buffer size
        size_t out_size = bz3_bound(input.size());
        result.data.resize(out_size);
        
        // Compress using bzip3
        int status = bz3_compress(
            block_size,
            input.data(),
            result.data.data(),
            input.size(),
            &out_size
        );
        
        if (status != BZ3_OK) {
            result.success = false;
            result.error_message = std::format("BZIP3 compression failed with error code: {}", status);
            return result;
        }
        
        // Resize to actual compressed size
        result.data.resize(out_size);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::format("BZIP3 compression error: {}", e.what());
    }
    
    return result;
}

CompressionResult Bzip2Compressor::decompress(std::span<const uint8_t> input) {
    CompressionResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (input.empty()) {
            result.success = false;
            result.error_message = "Empty input";
            return result;
        }
        
        // For decompression, bz3_decompress needs the EXACT output size
        // BZIP3 stores frame size in header, but we don't parse it here
        // Use a large buffer and let bzip3 tell us the actual size
        // Typical compression ratio: 5-20%, so 100x should be safe
        size_t estimated_size = std::max<size_t>(input.size() * 100, 1024 * 1024);  // At least 1MB
        result.data.resize(estimated_size);
        
        size_t out_size = estimated_size;
        int status = bz3_decompress(
            input.data(),
            result.data.data(),
            input.size(),
            &out_size
        );
        
        // If buffer too small, try with maximum possible size
        if (status == BZ3_ERR_DATA_TOO_BIG || status == BZ3_ERR_DATA_SIZE_TOO_SMALL) {
            estimated_size = input.size() * 1000;  // 1000x
            result.data.resize(estimated_size);
            out_size = estimated_size;
            
            status = bz3_decompress(
                input.data(),
                result.data.data(),
                input.size(),
                &out_size
            );
        }
        
        if (status != BZ3_OK) {
            result.success = false;
            result.error_message = std::format("BZIP3 decompression failed with error code: {}", status);
            return result;
        }
        
        // Resize to actual decompressed size
        result.data.resize(out_size);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::format("BZIP3 decompression error: {}", e.what());
    }
    
    return result;
}

// ============================================================================
// LzmaCompressor
// ============================================================================

CompressionResult LzmaCompressor::compress(
    std::span<const uint8_t> input,
    int level
) {
    CompressionResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        level = std::clamp(level, 1, 9);
        
        // LZMA properties
        lzma_stream strm = LZMA_STREAM_INIT;
        
        // Initialize encoder
        lzma_ret ret = lzma_easy_encoder(&strm, level, LZMA_CHECK_CRC64);
        if (ret != LZMA_OK) {
            result.success = false;
            result.error_message = "LZMA encoder initialization failed";
            return result;
        }
        
        // Allocate output buffer
        size_t out_size = lzma_stream_buffer_bound(input.size());
        result.data.resize(out_size);
        
        // Setup streams
        strm.next_in = input.data();
        strm.avail_in = input.size();
        strm.next_out = result.data.data();
        strm.avail_out = result.data.size();
        
        // Compress
        ret = lzma_code(&strm, LZMA_FINISH);
        
        if (ret != LZMA_STREAM_END) {
            lzma_end(&strm);
            result.success = false;
            result.error_message = fmt::format("LZMA compression failed: error {}", static_cast<int>(ret));
            return result;
        }
        
        size_t compressed_size = strm.total_out;
        lzma_end(&strm);
        
        result.data.resize(compressed_size);
        
        result.success = true;
        result.original_size = input.size();
        result.compressed_size = compressed_size;
        result.compression_ratio = 100.0 * (1.0 - static_cast<double>(compressed_size) / input.size());
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

CompressionResult LzmaCompressor::decompress(std::span<const uint8_t> input) {
    CompressionResult result;
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        lzma_stream strm = LZMA_STREAM_INIT;
        
        // Initialize decoder
        lzma_ret ret = lzma_stream_decoder(&strm, UINT64_MAX, LZMA_CONCATENATED);
        if (ret != LZMA_OK) {
            result.success = false;
            result.error_message = "LZMA decoder initialization failed";
            return result;
        }
        
        // Allocate output buffer
        result.data.resize(input.size() * 4);
        
        strm.next_in = input.data();
        strm.avail_in = input.size();
        strm.next_out = result.data.data();
        strm.avail_out = result.data.size();
        
        // Decompress, growing buffer if needed
        while (true) {
            // Use LZMA_FINISH for single-call decompression when all input is available
            lzma_action action = (strm.avail_in > 0) ? LZMA_RUN : LZMA_FINISH;
            ret = lzma_code(&strm, action);
            
            if (ret == LZMA_STREAM_END) {
                break;
            }
            
            if (ret == LZMA_OK && strm.avail_out == 0) {
                // Need more output space
                size_t old_size = result.data.size();
                result.data.resize(old_size * 2);
                strm.next_out = result.data.data() + old_size;
                strm.avail_out = old_size;
                continue;
            }
            
            if (ret != LZMA_OK) {
                lzma_end(&strm);
                result.success = false;
                result.error_message = fmt::format("LZMA decompression failed: error {}", static_cast<int>(ret));
                return result;
            }
        }
        
        size_t decompressed_size = strm.total_out;
        lzma_end(&strm);
        
        result.data.resize(decompressed_size);
        
        result.success = true;
        result.original_size = input.size();
        result.compressed_size = decompressed_size;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

} // namespace compression
} // namespace filevault
