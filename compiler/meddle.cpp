#include "core/logger.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "tree/ccgn.h"
#include "tree/nameres.h"
#include "tree/sema.h"
#include "tree/unit.h"

#include <boost/filesystem.hpp>

#include <chrono>
#include <cstdlib>
#include <vector>

using namespace meddle;

File parseInputFile(const String &path) {
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
    return File(filename, absol, buffer, directory);
}

int main(int argc, char **argv) {
    //if (argc < 2)
    //    fatal("no input files");

    auto start = std::chrono::high_resolution_clock::now();

    Options opts {
        .KeepCC = 1,
        .Time = 1,
    };
    std::vector<File> files;
    std::vector<TranslationUnit *> units;
    files.push_back(parseInputFile("samples/return_zero.mdl"));
    //files.push_back(File("test.mdl", "/", "/test.mdl", "main :: () i32 { fix x: i64 = 42; ret x; }"));

    Lexer lexer = Lexer(files[0]);
    TokenStream stream = lexer.unwrap(&opts);

    log("Lexed " + std::to_string(opts.lexedLines) + " lines across " + std::to_string(files.size()) + " file(s).");

    Parser parser = Parser(files[0], stream);
    units.push_back(parser.get());

    for (auto &unit : units) {
        NameResolution NR = NameResolution(opts, unit);
        Sema sema = Sema(opts, unit);
        CCGN ccgn = CCGN(opts, unit);
    }

    if (opts.Time) {
        auto frontend = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totalDuration = frontend - start;
        log("Frontend took: " + std::to_string(totalDuration.count()) + "s.");
    }

    String clang = "clang -c ";
    for (auto &unit : units)
        clang += unit->getFile().filename + ".c";

    std::system(clang.c_str());

    String ld = "clang -o " + opts.output + " ";
    for (auto &unit : units)
        ld += unit->getFile().filename + ".o";

    std::system(ld.c_str());

    for (auto &unit : units) {
        String file = unit->getFile().filename;
        String rm = "rm " + file + ".o ";
        if (!opts.KeepCC) 
            rm += file + ".c " + file + ".h";

        std::system(rm.c_str());
    }

    if (opts.Time) {
        auto frontend = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totalDuration = frontend - start;
        log("Backend took: " + std::to_string(totalDuration.count()) + "s.");
    }

    for (auto &unit : units)
        delete unit;

    files.clear();
    units.clear();
    return 0;
}
