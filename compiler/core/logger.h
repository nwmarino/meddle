#ifndef MEDDLE_LOGGER_H
#define MEDDLE_LOGGER_H

#include "metadata.h"

#include <iostream>

namespace meddle {

inline void info(const String &m, Metadata *md = nullptr) {
    if (md)
        std::cout << md->file.filename << ":" << md->line << ":" << md->col << ": ";
    else
        std::cout << "meddle: ";

    std::cout << "info: " << m << "\n";
}

inline void warn(const String &m, Metadata *md = nullptr) {
    if (md)
        std::cout << md->file.filename << ":" << md->line << ":" << md->col << ": ";
    else
        std::cout << "meddle: ";

    std::cout << "warning: " << m << "\n";
}

inline void fatal(const String &m, Metadata *md = nullptr) {
    if (md)
        std::cout << md->file.filename << ":" << md->line << ":" << md->col << ": ";
    else
        std::cout << "meddle: ";

    std::cout << "error: " << m << "\n";
    exit(1);
}

} // namespace meddle

#endif // MEDDLE_LOGGER_H
