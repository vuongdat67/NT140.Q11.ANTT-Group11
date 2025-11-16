#ifndef FILEVAULT_STEGANOGRAPHY_LSB_HPP
#define FILEVAULT_STEGANOGRAPHY_LSB_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <span>

namespace filevault::steganography {

/**
 * @brief LSB (Least Significant Bit) Steganography
 * 
 * Embeds secret data into image pixels by modifying the least significant bits.
 * Supports PNG and BMP images.
 * 
 * Capacity: For RGB images, ~3 bits per pixel (if using 1 bit per channel)
 *           For an 800x600 image: (800 * 600 * 3) / 8 = 180,000 bytes max
 */
class LSBSteganography {
public:
    /**
     * @brief Embed secret data into image
     * 
     * @param cover_image_path Path to cover image (PNG/BMP)
     * @param secret_data Data to hide
     * @param output_path Path for output stego image
     * @param bits_per_channel Number of LSBs to use per color channel (1-4, default 1)
     * @return true if successful, false otherwise
     */
    static bool embed(
        const std::string& cover_image_path,
        std::span<const uint8_t> secret_data,
        const std::string& output_path,
        int bits_per_channel = 1
    );
    
    /**
     * @brief Extract hidden data from stego image
     * 
     * @param stego_image_path Path to stego image
     * @param bits_per_channel Number of LSBs used per color channel (must match embed)
     * @return Extracted secret data, or empty vector on failure
     */
    static std::vector<uint8_t> extract(
        const std::string& stego_image_path,
        int bits_per_channel = 1
    );
    
    /**
     * @brief Calculate maximum capacity for given image
     * 
     * @param image_path Path to image
     * @param bits_per_channel Number of LSBs per channel
     * @return Maximum bytes that can be hidden, or 0 on error
     */
    static size_t calculate_capacity(
        const std::string& image_path,
        int bits_per_channel = 1
    );
    
private:
    // Embed length header (4 bytes) at the beginning
    static constexpr size_t LENGTH_HEADER_SIZE = 4;
    
    // Helper to embed one byte into image data
    static void embed_byte(
        uint8_t* pixel_data,
        size_t pixel_count,
        size_t& bit_index,
        uint8_t byte,
        int bits_per_channel
    );
    
    // Helper to extract one byte from image data
    static uint8_t extract_byte(
        const uint8_t* pixel_data,
        size_t pixel_count,
        size_t& bit_index,
        int bits_per_channel
    );
};

} // namespace filevault::steganography

#endif // FILEVAULT_STEGANOGRAPHY_LSB_HPP
