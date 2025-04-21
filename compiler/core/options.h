#ifndef MEDDLE_OPTIONS_H
#define MEDDLE_OPTIONS_H

namespace meddle {

struct Options final {
    unsigned lexedLines = 0;

    unsigned Debug:1;
    unsigned Time:1;

    Options() {};
};

} // namespace meddle

#endif // MEDDLE_OPTIONS_H
