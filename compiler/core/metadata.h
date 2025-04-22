#ifndef MEDDLE_METADATA_H
#define MEDDLE_METADATA_H

#include <string>

using String = std::string;

namespace meddle {

struct File final {
    String filename;
    String dir;
    String path;
    String contents;

    File(
        const String &filename, 
        const String &dir, 
        const String &path, 
        const String &contents
    ) : filename(filename), dir(dir), path(path), contents(contents) {}
};

struct Metadata final {
    File file;
    unsigned line;
    unsigned col;

    Metadata(const File &file) : file(file), line(1), col(1) {}
    
    Metadata(const File &file, unsigned line, unsigned col) 
      : file(file), line(line), col(col) {}
};

} // namespace meddle

#endif // MEDDLE_METADATA_H