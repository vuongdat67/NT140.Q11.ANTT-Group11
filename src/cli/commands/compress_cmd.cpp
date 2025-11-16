#include "filevault/cli/commands/compress_cmd.hpp"
#include "filevault/compression/compressor.hpp"
#include "filevault/utils/file_io.hpp"
#include "filevault/utils/console.hpp"
#include <fmt/core.h>
#include <chrono>
#include <fstream>

namespace filevault::cli {

void CompressCommand::setup(CLI::App& app) {
    subcommand_ = app.add_subcommand(name(), description());
    
    subcommand_->add_option("input", input_file_, "Input file")
        ->required()
        ->check(CLI::ExistingFile);
    
    subcommand_->add_option("output,-o,--output", output_file_, "Output file");
    
    subcommand_->add_option("-a,--algorithm", algorithm_, "Compression algorithm")
        ->check(CLI::IsMember({"zlib", "bzip2", "lzma"}));
    
    subcommand_->add_option("-l,--level", level_, "Compression level (1-9)")
        ->check(CLI::Range(1, 9));
    
    subcommand_->add_flag("-d,--decompress", decompress_, 
                  "Decompress mode");
    
    subcommand_->add_flag("--auto-detect", auto_detect_,
                  "Auto-detect compression algorithm");
    
    subcommand_->add_flag("-v,--verbose", verbose_, "Verbose output");
    
    subcommand_->add_flag("--benchmark", benchmark_, 
                  "Show compression ratio and timing");
    
    subcommand_->callback([this]() { execute(); });
}

int CompressCommand::execute() {
    // Generate output path if not provided
    if (output_file_.empty()) {
        output_file_ = generate_output_path(input_file_, !decompress_);
    }
    
    if (decompress_) {
        return do_decompress();
    } else {
        return do_compress();
    }
}

int CompressCommand::do_compress() {
    utils::Console::separator();
    fmt::print("\n{:^80}\n", "FileVault Compression");
    utils::Console::separator();
    
    // Show settings
    utils::Console::info(fmt::format("Input:     {}", input_file_));
    utils::Console::info(fmt::format("Output:    {}", output_file_));
    utils::Console::info(fmt::format("Algorithm: {}", algorithm_));
    utils::Console::info(fmt::format("Level:     {}", level_));
    utils::Console::separator();
    
    // Read input file
    auto file_result = utils::FileIO::read_file(input_file_);
    if (!file_result) {
        utils::Console::error(file_result.error_message);
        return 1;
    }
    
    auto plaintext = file_result.value;
    size_t original_size = plaintext.size();
    utils::Console::info(fmt::format("Read {} bytes", original_size));
    
    // Parse algorithm
    auto comp_type = compression::CompressionService::parse_algorithm(algorithm_);
    if (comp_type == core::CompressionType::NONE) {
        utils::Console::error(fmt::format("Unknown algorithm: {}", algorithm_));
        return 1;
    }
    
    // Create compressor
    auto compressor = compression::CompressionService::create(comp_type);
    if (!compressor) {
        utils::Console::error("Failed to create compressor");
        return 1;
    }
    
    // Compress
    utils::Console::info("Compressing...");
    auto start = std::chrono::high_resolution_clock::now();
    
    auto compress_result = compressor->compress(plaintext, level_);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    
    if (!compress_result.success) {
        utils::Console::error(compress_result.error_message);
        return 1;
    }
    
    // Write output
    auto write_result = utils::FileIO::write_file(output_file_, compress_result.data);
    if (!write_result) {
        utils::Console::error(write_result.error_message);
        return 1;
    }
    
    // Show results
    utils::Console::separator();
    utils::Console::success("Compression completed!");
    
    size_t compressed_size = compress_result.data.size();
    double ratio = (double)compressed_size / original_size * 100.0;
    double throughput = (original_size / 1024.0 / 1024.0) / (duration.count() / 1000.0);
    
    utils::Console::info(fmt::format("Output:      {} ({} bytes)", 
                                     output_file_, compressed_size));
    utils::Console::info(fmt::format("Original:    {} bytes", original_size));
    utils::Console::info(fmt::format("Compressed:  {} bytes ({:.1f}%)", 
                                     compressed_size, ratio));
    
    if (benchmark_) {
        utils::Console::info(fmt::format("Time:        {:.2f} ms", duration.count()));
        utils::Console::info(fmt::format("Throughput:  {:.2f} MB/s", throughput));
    }
    
    return 0;
}

int CompressCommand::do_decompress() {
    utils::Console::separator();
    fmt::print("\n{:^80}\n", "FileVault Decompression");
    utils::Console::separator();
    
    // Auto-detect algorithm if requested
    if (auto_detect_) {
        algorithm_ = detect_algorithm(input_file_);
        if (algorithm_.empty()) {
            utils::Console::error("Failed to auto-detect compression algorithm");
            utils::Console::info("Try specifying algorithm with -a/--algorithm");
            return 1;
        }
        utils::Console::info(fmt::format("Detected algorithm: {}", algorithm_));
    }
    
    // Show settings
    utils::Console::info(fmt::format("Input:     {}", input_file_));
    utils::Console::info(fmt::format("Output:    {}", output_file_));
    utils::Console::info(fmt::format("Algorithm: {}", algorithm_));
    utils::Console::separator();
    
    // Read compressed file
    auto file_result = utils::FileIO::read_file(input_file_);
    if (!file_result) {
        utils::Console::error(file_result.error_message);
        return 1;
    }
    
    auto compressed_data = file_result.value;
    size_t compressed_size = compressed_data.size();
    utils::Console::info(fmt::format("Read {} bytes", compressed_size));
    
    // Parse algorithm
    auto comp_type = compression::CompressionService::parse_algorithm(algorithm_);
    if (comp_type == core::CompressionType::NONE) {
        utils::Console::error(fmt::format("Unknown algorithm: {}", algorithm_));
        return 1;
    }
    
    // Create compressor
    auto compressor = compression::CompressionService::create(comp_type);
    if (!compressor) {
        utils::Console::error("Failed to create compressor");
        return 1;
    }
    
    // Decompress
    utils::Console::info("Decompressing...");
    auto start = std::chrono::high_resolution_clock::now();
    
    auto decompress_result = compressor->decompress(compressed_data);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    
    if (!decompress_result.success) {
        utils::Console::error(decompress_result.error_message);
        return 1;
    }
    
    // Write output
    auto write_result = utils::FileIO::write_file(output_file_, decompress_result.data);
    if (!write_result) {
        utils::Console::error(write_result.error_message);
        return 1;
    }
    
    // Show results
    utils::Console::separator();
    utils::Console::success("Decompression completed!");
    
    size_t decompressed_size = decompress_result.data.size();
    double ratio = (double)compressed_size / decompressed_size * 100.0;
    double throughput = (decompressed_size / 1024.0 / 1024.0) / (duration.count() / 1000.0);
    
    utils::Console::info(fmt::format("Output:        {} ({} bytes)", 
                                     output_file_, decompressed_size));
    utils::Console::info(fmt::format("Compressed:    {} bytes", compressed_size));
    utils::Console::info(fmt::format("Decompressed:  {} bytes ({:.1f}%)", 
                                     decompressed_size, ratio));
    
    if (benchmark_) {
        utils::Console::info(fmt::format("Time:          {:.2f} ms", duration.count()));
        utils::Console::info(fmt::format("Throughput:    {:.2f} MB/s", throughput));
    }
    
    return 0;
}

std::string CompressCommand::detect_algorithm(const std::string& path) {
    // Read first few bytes to detect magic numbers
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return "";
    }
    
    unsigned char magic[4] = {0};
    file.read(reinterpret_cast<char*>(magic), 4);
    
    // zlib: 78 9C (default compression) or 78 01 (no compression) or 78 DA (best compression)
    if (magic[0] == 0x78 && (magic[1] == 0x9C || magic[1] == 0x01 || magic[1] == 0xDA)) {
        return "zlib";
    }
    
    // bzip2: "BZ" header
    if (magic[0] == 0x42 && magic[1] == 0x5A) {
        return "bzip2";
    }
    
    // LZMA: FD 37 7A 58 5A 00
    if (magic[0] == 0xFD && magic[1] == 0x37 && magic[2] == 0x7A && magic[3] == 0x58) {
        return "lzma";
    }
    
    // Try extension fallback
    if (path.ends_with(".zlib") || path.ends_with(".zz")) {
        return "zlib";
    } else if (path.ends_with(".bz2")) {
        return "bzip2";
    } else if (path.ends_with(".xz") || path.ends_with(".lzma")) {
        return "lzma";
    }
    
    return "";
}

std::string CompressCommand::generate_output_path(const std::string& input, bool compressing) {
    if (compressing) {
        // Add algorithm extension
        if (algorithm_ == "zlib") {
            return input + ".zlib";
        } else if (algorithm_ == "bzip2") {
            return input + ".bz2";
        } else if (algorithm_ == "lzma") {
            return input + ".xz";
        }
        return input + ".compressed";
    } else {
        // Remove compression extension
        std::string output = input;
        
        // Remove common compression extensions
        if (output.ends_with(".zlib")) {
            return output.substr(0, output.length() - 5);
        } else if (output.ends_with(".bz2")) {
            return output.substr(0, output.length() - 4);
        } else if (output.ends_with(".xz")) {
            return output.substr(0, output.length() - 3);
        } else if (output.ends_with(".lzma")) {
            return output.substr(0, output.length() - 5);
        } else if (output.ends_with(".zz")) {
            return output.substr(0, output.length() - 3);
        } else if (output.ends_with(".compressed")) {
            return output.substr(0, output.length() - 11);
        }
        
        // Just add .decompressed
        return input + ".decompressed";
    }
}

} // namespace filevault::cli
