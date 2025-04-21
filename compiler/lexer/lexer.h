#ifndef MEDDLE_LEXER_H
#define MEDDLE_LEXER_H

#include "tokenstream.h"

using String = std::string;

namespace meddle {

/// A lexer which produces a token stream from a source file.
class Lexer final {
    TokenStream m_Stream;
    String m_Buffer;
    Metadata m_Loc;
    unsigned long m_Iter = 0;

    bool isEof() const { return m_Iter >= m_Buffer.size(); }

    char next(unsigned long n = 1) const {
        if (m_Iter + n >= m_Buffer.size())
            return '\0'; 
        
        return m_Buffer[m_Iter + n]; 
    }

    char curr() const { return next(0); }

    void move(unsigned long n = 1) {
        m_Iter += n;
        m_Loc.col++;
    }

    Token lex();

public:
    Lexer(const File &file);

    TokenStream unwrap() const { return m_Stream; }
};

} // namespace meddle

#endif // MEDDLE_LEXER_H
