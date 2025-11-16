#ifndef FILEVAULT_CORE_RESULT_HPP
#define FILEVAULT_CORE_RESULT_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include "types.hpp"

namespace filevault {
namespace core {

/**
 * @brief Result of cryptographic operations
 */
struct CryptoResult {
    bool success = false;
    std::string error_message;
    std::vector<uint8_t> data;
    
    // Metadata
    AlgorithmType algorithm_used;
    size_t original_size = 0;
    size_t final_size = 0;
    double processing_time_ms = 0.0;
    
    // Additional info
    std::optional<std::vector<uint8_t>> salt;
    std::optional<std::vector<uint8_t>> nonce;
    std::optional<std::vector<uint8_t>> tag;
};

/**
 * @brief Generic result type
 */
template<typename T>
struct Result {
    bool success = false;
    std::string error_message;
    T value;
    
    // Success constructor
    static Result<T> ok(T val) {
        Result<T> r;
        r.success = true;
        r.value = std::move(val);
        return r;
    }
    
    // Error constructor
    static Result<T> error(const std::string& msg) {
        Result<T> r;
        r.success = false;
        r.error_message = msg;
        return r;
    }
    
    // Check if result is ok
    explicit operator bool() const { return success; }
    
    // Get value (throws if error)
    T& unwrap() {
        if (!success) {
            throw std::runtime_error("Attempted to unwrap error: " + error_message);
        }
        return value;
    }
};

/**
 * @brief Specialization for void
 */
template<>
struct Result<void> {
    bool success = false;
    std::string error_message;
    
    static Result<void> ok() {
        Result<void> r;
        r.success = true;
        return r;
    }
    
    static Result<void> error(const std::string& msg) {
        Result<void> r;
        r.success = false;
        r.error_message = msg;
        return r;
    }
    
    explicit operator bool() const { return success; }
};

} // namespace core
} // namespace filevault

#endif // FILEVAULT_CORE_RESULT_HPP
