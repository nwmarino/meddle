#ifndef MEDDLE_TOKENSTREAM_H
#define MEDDLE_TOKENSTREAM_H

#include "token.h"

#include <cassert>
#include <iostream>
#include <vector>

namespace meddle {

class TokenStream final { 
    std::vector<Token> m_Buffer;
    unsigned m_Iter = 0;

public:
    TokenStream() : m_Buffer({}) {}
    TokenStream(std::vector<Token> buf) : m_Buffer(std::move(buf)) {}

    const std::vector<Token> &getTokens() const { return m_Buffer; }

    /// Get the current token and move the cursor.
    const Token *get() {
        assert(m_Iter < m_Buffer.size());
        return &m_Buffer.at(m_Iter++);
    }

    /// Get the token at \p pos without moving the cursor.
    const Token *get(unsigned pos) const {
        assert(pos < m_Buffer.size());
        return &m_Buffer.at(pos);
    }

    bool isEnd() const { return m_Iter >= m_Buffer.size(); }

    unsigned getPos() const { return m_Iter; }

    void setPos(unsigned pos) { m_Iter = pos; }

    void add(Token token) { m_Buffer.push_back(token); }
};

} // namespace meddle

#endif // MEDDLE_TOKENSTREAM_H
