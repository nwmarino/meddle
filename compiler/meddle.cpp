#include "cgn/codegen.h"
#include "core/logger.h"
#include "core/metadata.h"
#include "lexer/lexer.h"
#include "mir/segment.h"
#include "parser/parser.h"
#include "tree/unit.h"
#include "tree/unitman.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <vector>

using namespace meddle;
using mir::Target;

int main(int argc, char **argv) {
    //if (argc < 2)
    //    fatal("no input files");

    auto start = std::chrono::high_resolution_clock::now();

    Options opts {
        .KeepCC = 1,
        .NamedMIR = 0,
        .Time = 1,
    };

    std::vector<File> files;
    UnitManager units;
    std::vector<mir::Segment *> segments;

    files.push_back(parseInputFile("samples/foo.mdl"));
    files.push_back(parseInputFile("samples/bar.mdl"));

    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        Parser parser = Parser(file, lexer.unwrap(&opts));
        units.addUnit(parser.get());
    }

    log("Lexed " + std::to_string(opts.lexedLines) + " lines across " + 
        std::to_string(files.size()) + " file(s).");

    units.drive(opts);

    Target target = Target(
        mir::Arch::X86_64, 
        mir::OS::Linux, 
        mir::ABI::SystemV
    );

    for (auto &unit : units.getUnits()) {
        mir::Segment *seg = new mir::Segment(target);
        assert(seg && "Unable to create segment.");
        CGN *cgn = new CGN(opts, unit, seg);
        segments.push_back(seg);

        std::ofstream OS = std::ofstream(unit->getFile().filename + ".mir");
        seg->print(OS);

        delete cgn;
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> frontend;
    if (opts.Time) {
        frontend = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totalDuration = frontend - start;
        log("  Frontend took: " + std::to_string(totalDuration.count()) + "s.");
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> backend;
    if (opts.Time) {
        backend = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totalDuration = backend - frontend;
        log("  Backend took: " + std::to_string(totalDuration.count()) + "s.");
    }

    if (opts.Time) {
        auto total = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totalDuration = total - start;
        log("Compilation took: " + std::to_string(totalDuration.count()) + "s.");
    }

    for (auto &seg : segments)
        delete seg;

    files.clear();
    return 0;
}
