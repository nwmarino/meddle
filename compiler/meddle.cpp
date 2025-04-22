#include "core/logger.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "tree/ccgn.h"
#include "tree/nameres.h"
#include "tree/sema.h"
#include "tree/unit.h"

#include <chrono>
#include <vector>

using namespace meddle;

int main(int argc, char **argv) {
    //if (argc < 2)
    //    fatal("no input files");

    auto start = std::chrono::high_resolution_clock::now();

    Options opts;
    std::vector<File> files;
    files.push_back(File("test.mdl", "/", "/test.mdl", "main :: () i32 { ret 0; }"));

    Lexer lexer = Lexer(files[0]);
    TokenStream stream = lexer.unwrap(&opts);

    info("Lexed " + std::to_string(opts.lexedLines) + " lines across " + std::to_string(files.size()) + " file(s).");

    Parser parser = Parser(files[0], stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(opts, unit);
    Sema sema = Sema(opts, unit);
    CCGN ccgn = CCGN(opts, unit);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> totalDuration = end - start;
    info("Finished compiling in " + std::to_string(totalDuration.count()) + " seconds.");
    
    delete unit;
    return 0;
}
