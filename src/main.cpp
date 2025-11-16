#include "filevault/cli/app.hpp"
#include <iostream>
#include <exception>

int main(int argc, char** argv) {
    try {
        filevault::cli::Application app;
        app.initialize();
        return app.run(argc, argv);
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error" << std::endl;
        return 1;
    }
}
