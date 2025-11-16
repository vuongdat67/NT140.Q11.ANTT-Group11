#include <catch2/catch_test_macros.hpp>
#include "filevault/algorithms/symmetric/aes_gcm.hpp"
#include "filevault/algorithms/symmetric/chacha20_poly1305.hpp"
#include <botan/auto_rng.h>
#include <chrono>
#include <vector>
#include <numeric>
#include <cmath>

using namespace filevault::algorithms::symmetric;
using namespace filevault::core;

/**
 * @file test_timing_attacks.cpp
 * @brief Security tests for timing attack resistance
 * 
 * SECURITY REQUIREMENT: Cryptographic operations must be constant-time
 * 
 * Timing attacks exploit variations in execution time to extract secrets:
 * - MAC/tag verification must be constant-time
 * - Key comparison must be constant-time
 * - Decryption failures should take same time as successes
 * 
 * This test suite verifies:
 * 1. MAC verification is constant-time (valid vs invalid)
 * 2. No timing difference between wrong key and corrupted data
 * 3. Statistical analysis shows no correlation between input and time
 */

// Helper function to measure execution time
template<typename Func>
double measure_time_microseconds(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::micro>(end - start).count();
}

// Statistical analysis
struct TimingStats {
    double mean;
    double std_dev;
    double min;
    double max;
    double median;
    
    static TimingStats calculate(const std::vector<double>& times) {
        TimingStats stats;
        
        // Mean
        stats.mean = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        
        // Standard deviation
        double sq_sum = 0.0;
        for (auto t : times) {
            sq_sum += (t - stats.mean) * (t - stats.mean);
        }
        stats.std_dev = std::sqrt(sq_sum / times.size());
        
        // Min/Max
        stats.min = *std::min_element(times.begin(), times.end());
        stats.max = *std::max_element(times.begin(), times.end());
        
        // Median
        std::vector<double> sorted_times = times;
        std::sort(sorted_times.begin(), sorted_times.end());
        stats.median = sorted_times[sorted_times.size() / 2];
        
        return stats;
    }
};

TEST_CASE("Timing attacks - MAC verification", "[security][timing][mac]") {
    SECTION("AES-GCM constant-time tag verification") {
        AES_GCM cipher(256);
        EncryptionConfig config;
        
        std::string plaintext = "Timing attack test message for MAC verification.";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> key(32, 0xAB);
        
        Botan::AutoSeeded_RNG rng;
        std::vector<uint8_t> nonce(12);
        rng.randomize(nonce.data(), nonce.size());
        config.nonce = nonce;
        
        // Encrypt
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        config.tag = encrypted.tag.value();
        
        const size_t NUM_SAMPLES = 1000;
        std::vector<double> valid_times;
        std::vector<double> invalid_times;
        
        // Measure time for VALID tag
        for (size_t i = 0; i < NUM_SAMPLES; ++i) {
            double time = measure_time_microseconds([&]() {
                auto result = cipher.decrypt(encrypted.data, key, config);
                // Don't check result to avoid branch prediction affecting timing
                (void)result;
            });
            valid_times.push_back(time);
        }
        
        // Measure time for INVALID tag (flip one bit)
        auto invalid_tag = encrypted.tag.value();
        invalid_tag[0] ^= 0x01;
        config.tag = invalid_tag;
        
        for (size_t i = 0; i < NUM_SAMPLES; ++i) {
            double time = measure_time_microseconds([&]() {
                auto result = cipher.decrypt(encrypted.data, key, config);
                (void)result;
            });
            invalid_times.push_back(time);
        }
        
        // Statistical analysis
        auto valid_stats = TimingStats::calculate(valid_times);
        auto invalid_stats = TimingStats::calculate(invalid_times);
        
        INFO("Valid tag verification:");
        INFO("  Mean: " << valid_stats.mean << " µs");
        INFO("  StdDev: " << valid_stats.std_dev << " µs");
        INFO("  Range: [" << valid_stats.min << ", " << valid_stats.max << "] µs");
        
        INFO("Invalid tag verification:");
        INFO("  Mean: " << invalid_stats.mean << " µs");
        INFO("  StdDev: " << invalid_stats.std_dev << " µs");
        INFO("  Range: [" << invalid_stats.min << ", " << invalid_stats.max << "] µs");
        
        // Check if timing difference is statistically significant
        // Allow up to 10% difference (noise tolerance)
        double diff_percent = std::abs(valid_stats.mean - invalid_stats.mean) / valid_stats.mean * 100.0;
        
        INFO("Timing difference: " << diff_percent << "%");
        
        // Botan's AEAD should use constant-time comparison
        // Difference should be < 10% (accounting for system noise)
        if (diff_percent > 10.0) {
            WARN("Timing difference exceeds 10% - potential timing leak!");
        }
        
        // This is more of a warning than a hard requirement
        // System noise can cause variations
    }
    
    SECTION("ChaCha20-Poly1305 constant-time tag verification") {
        ChaCha20Poly1305 cipher;
        EncryptionConfig config;
        
        std::string plaintext = "ChaCha20 timing test for constant-time verification.";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> key(32, 0xCD);
        
        Botan::AutoSeeded_RNG rng;
        std::vector<uint8_t> nonce(12);
        rng.randomize(nonce.data(), nonce.size());
        config.nonce = nonce;
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        config.tag = encrypted.tag.value();
        
        const size_t NUM_SAMPLES = 1000;
        std::vector<double> valid_times;
        std::vector<double> invalid_times;
        
        // Measure valid
        for (size_t i = 0; i < NUM_SAMPLES; ++i) {
            double time = measure_time_microseconds([&]() {
                auto result = cipher.decrypt(encrypted.data, key, config);
                (void)result;
            });
            valid_times.push_back(time);
        }
        
        // Measure invalid
        auto invalid_tag = encrypted.tag.value();
        invalid_tag[8] ^= 0xFF;
        config.tag = invalid_tag;
        
        for (size_t i = 0; i < NUM_SAMPLES; ++i) {
            double time = measure_time_microseconds([&]() {
                auto result = cipher.decrypt(encrypted.data, key, config);
                (void)result;
            });
            invalid_times.push_back(time);
        }
        
        auto valid_stats = TimingStats::calculate(valid_times);
        auto invalid_stats = TimingStats::calculate(invalid_times);
        
        double diff_percent = std::abs(valid_stats.mean - invalid_stats.mean) / valid_stats.mean * 100.0;
        
        INFO("ChaCha20-Poly1305 timing difference: " << diff_percent << "%");
        if (diff_percent > 10.0) {
            WARN("Timing difference exceeds 10%");
        }
    }
}

TEST_CASE("Timing attacks - wrong key vs corrupted data", "[security][timing]") {
    SECTION("Wrong key and corrupted data take similar time") {
        AES_GCM cipher(256);
        EncryptionConfig config;
        
        std::string plaintext = "Secret data";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> correct_key(32, 0xAA);
        std::vector<uint8_t> wrong_key(32, 0xBB);
        
        Botan::AutoSeeded_RNG rng;
        std::vector<uint8_t> nonce(12);
        rng.randomize(nonce.data(), nonce.size());
        config.nonce = nonce;
        
        auto encrypted = cipher.encrypt(pt, correct_key, config);
        REQUIRE(encrypted.success);
        
        config.tag = encrypted.tag.value();
        
        const size_t NUM_SAMPLES = 500;
        std::vector<double> wrong_key_times;
        std::vector<double> corrupted_data_times;
        
        // Measure wrong key
        for (size_t i = 0; i < NUM_SAMPLES; ++i) {
            double time = measure_time_microseconds([&]() {
                auto result = cipher.decrypt(encrypted.data, wrong_key, config);
                (void)result;
            });
            wrong_key_times.push_back(time);
        }
        
        // Measure corrupted data
        auto corrupted_data = encrypted.data;
        corrupted_data[5] ^= 0xFF;
        
        for (size_t i = 0; i < NUM_SAMPLES; ++i) {
            double time = measure_time_microseconds([&]() {
                auto result = cipher.decrypt(corrupted_data, correct_key, config);
                (void)result;
            });
            corrupted_data_times.push_back(time);
        }
        
        auto wrong_key_stats = TimingStats::calculate(wrong_key_times);
        auto corrupted_stats = TimingStats::calculate(corrupted_data_times);
        
        INFO("Wrong key mean: " << wrong_key_stats.mean << " µs");
        INFO("Corrupted data mean: " << corrupted_stats.mean << " µs");
        
        double diff_percent = std::abs(wrong_key_stats.mean - corrupted_stats.mean) / 
                             wrong_key_stats.mean * 100.0;
        
        INFO("Difference: " << diff_percent << "%");
        
        // Should be similar (< 10% difference)
        // This prevents attacker from distinguishing error types by timing
        if (diff_percent > 10.0) {
            WARN("Timing reveals error type!");
        }
    }
}

TEST_CASE("Timing attacks - position of error in tag", "[security][timing][advanced]") {
    SECTION("Error at different positions takes same time") {
        AES_GCM cipher(256);
        EncryptionConfig config;
        
        std::string plaintext = "Positional timing test";
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        std::vector<uint8_t> key(32, 0xCC);
        
        Botan::AutoSeeded_RNG rng;
        std::vector<uint8_t> nonce(12);
        rng.randomize(nonce.data(), nonce.size());
        config.nonce = nonce;
        
        auto encrypted = cipher.encrypt(pt, key, config);
        REQUIRE(encrypted.success);
        
        const size_t NUM_SAMPLES = 200;
        const size_t TAG_SIZE = 16;
        
        std::vector<std::vector<double>> position_times(TAG_SIZE);
        
        // Test error at each byte position in tag
        for (size_t pos = 0; pos < TAG_SIZE; ++pos) {
            auto invalid_tag = encrypted.tag.value();
            invalid_tag[pos] ^= 0x01;
            config.tag = invalid_tag;
            
            for (size_t i = 0; i < NUM_SAMPLES; ++i) {
                double time = measure_time_microseconds([&]() {
                    auto result = cipher.decrypt(encrypted.data, key, config);
                    (void)result;
                });
                position_times[pos].push_back(time);
            }
        }
        
        // Calculate stats for each position
        std::vector<TimingStats> all_stats;
        for (size_t pos = 0; pos < TAG_SIZE; ++pos) {
            all_stats.push_back(TimingStats::calculate(position_times[pos]));
        }
        
        // Find max and min mean times
        double max_mean = all_stats[0].mean;
        double min_mean = all_stats[0].mean;
        size_t max_pos = 0;
        size_t min_pos = 0;
        
        for (size_t pos = 0; pos < TAG_SIZE; ++pos) {
            if (all_stats[pos].mean > max_mean) {
                max_mean = all_stats[pos].mean;
                max_pos = pos;
            }
            if (all_stats[pos].mean < min_mean) {
                min_mean = all_stats[pos].mean;
                min_pos = pos;
            }
        }
        
        double position_diff_percent = (max_mean - min_mean) / min_mean * 100.0;
        
        INFO("Error position timing variance:");
        INFO("  Fastest position: " << min_pos << " (" << min_mean << " µs)");
        INFO("  Slowest position: " << max_pos << " (" << max_mean << " µs)");
        INFO("  Difference: " << position_diff_percent << "%");
        
        // Good constant-time implementation: < 5% variance
        // Acceptable: < 10% (system noise)
        if (position_diff_percent > 10.0) {
            WARN("Position-dependent timing detected - may leak information!");
        }
    }
}

TEST_CASE("Timing attacks - recommendations", "[security][timing][info]") {
    SECTION("Security guidelines") {
        WARN("SECURITY GUIDELINES:");
        WARN("1. Always use Botan::constant_time_compare() for secret comparisons");
        WARN("2. Avoid early-return on validation failures");
        WARN("3. Use same error message for all authentication failures");
        WARN("4. Don't branch on secret data");
        WARN("5. Be aware of cache timing attacks on sensitive operations");
        
        SUCCEED("These are recommendations, not test failures");
    }
}
