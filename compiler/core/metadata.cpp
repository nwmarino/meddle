#include "logger.h"
#include "metadata.h"

#include <boost/filesystem.hpp>

#include <fstream>

using namespace meddle;

File meddle::parseInputFile(const String &path) {
    String absol;

    try {
        absol = boost::filesystem::canonical(path).string();
    } catch (const boost::filesystem::filesystem_error &e) {
        fatal("file does not exist: " + path, nullptr);
    }

    String buffer;
    try {
        unsigned size = boost::filesystem::file_size(path);
        buffer.resize(size);

        std::ifstream file(path, std::ios::binary);
        if (!file)
            fatal("failed to open file: " + path, nullptr);

        file.read(&buffer[0], size);
        
        if (file.gcount() != static_cast<std::streamsize>(size))
            fatal("failed to read entire file: " + path, nullptr);
    } catch (const std::exception &e) {
        fatal("failed to parse input file: " + String(e.what()), nullptr);
    }

    String filename = boost::filesystem::path(path).filename().string();
    String directory = boost::filesystem::path(path).parent_path().string();
    return File(filename, directory, absol, buffer);
}
