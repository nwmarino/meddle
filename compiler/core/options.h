#ifndef MEDDLE_OPTIONS_H
#define MEDDLE_OPTIONS_H

#include <string>

using String = std::string;

namespace meddle {

struct Options final {
    unsigned lexedLines = 0;
    String output = "main";

    unsigned Debug:1;
    unsigned KeepCC:1;
    unsigned Time:1;
};

} // namespace meddle

#endif // MEDDLE_OPTIONS_H
