#include "lexer.h"
#include "token.h"
#include <cctype>
#include <iostream>

using namespace meddle;

Lexer::Lexer(const File &file) 
  : m_Stream(), m_Buffer(file.contents), m_Loc(Metadata(file)) {
    for (;;) {
        if (m_Iter >= m_Buffer.size())
            break;

        m_Stream.add(lex());
    }

    m_Stream.add(Token(m_Loc));
}

Token Lexer::lex() {
    if (curr() == '\0') {
        return Token(m_Loc);
    } else if (curr() == '\n') {
        move();
        m_Loc.line++;
        m_Loc.col = 1;
        return lex();
    } else if (curr() == ' ' || curr() == '\t') {
        while (curr() == ' ' || curr() == '\t')
            move();

        return lex();
    }

    TokenKind kind;
    LiteralKind literal = LiteralKind::None;
    String value = "";
    Metadata start = m_Loc;

    switch (curr()) {
        case '+':
            if (next() == '+') {
                kind = TokenKind::PlusPlus;
                move(2);
            } else if (next() == '=') {
                kind = TokenKind::PlusEquals;
                move(2);
            } else {
                kind = TokenKind::Plus;
                move();
            }

            break;

        case '-':
            if (isdigit(next())) {
                goto DEFAULT;
            } else if (next() == '-') {
                kind = TokenKind::MinusMinus;
                move(2);
            } else if (next() == '=') {
                kind = TokenKind::MinusEquals;
                move(2);
            } else if (next() == '>') {
                kind = TokenKind::Arrow;
                move(2);
            } else {
                kind = TokenKind::Minus;
                move();
            }

            break;

        case '*':
            if (next() == '=') {
                kind = TokenKind::StarEquals;
                move(2);
            } else {
                kind = TokenKind::Star;
                move();
            }

            break;

        case '/':
            if (next() == '/') {
                kind = TokenKind::SlashSlash;
                move(2);
                while (curr() != '\n' && curr() != '\0')
                    move();
                
                return lex();
            } else if (next() == '=') {
                kind = TokenKind::SlashEquals;
                move(2);
            } else {
                kind = TokenKind::Slash;
                move();
            }
            
            break;

        case '<':
            if (next() == '<') {
                if (next(2) == '=') {
                    kind = TokenKind::LeftLeftEquals;
                    move(3);
                } else {
                    kind = TokenKind::LeftLeft;
                    move(2);
                }
            } else if (next() == '=') {
                kind = TokenKind::LeftEquals;
                move(2);
            } else {
                kind = TokenKind::Left;
                move();
            }

            break;

        case '>':
            if (next() == '>') {
                if (next(2) == '=') {
                    kind = TokenKind::RightRightEquals;
                    move(3);
                } else {
                    kind = TokenKind::RightRight;
                    move(2);
                }
            } else if (next() == '=') {
                kind = TokenKind::RightEquals;
                move(2);
            } else {
                kind = TokenKind::Right;
                move();
            }

            break;

        case '&':
            if (next() == '&') {
                kind = TokenKind::AndAnd;
                move(2);
            } else if (next() == '=') {
                kind = TokenKind::AndEquals;
                move(2);
            } else {
                kind = TokenKind::And;
                move();
            }

            break;

        case '|':
            if (next() == '|') {
                kind = TokenKind::OrOr;
                move(2);
            } else if (next() == '=') {
                kind = TokenKind::OrEquals;
                move(2);
            } else {
                kind = TokenKind::Or;
                move();
            }

            break;

        case '^':
            if (next() == '=') {
                kind = TokenKind::XorEquals;
                move(2);
            } else {
                kind = TokenKind::Xor;
                move();
            }

            break;

        case '%':
            if (next() == '=') {
                kind = TokenKind::PercentEquals;
                move(2);
            } else {
                kind = TokenKind::Percent;
                move();
            }

            break;

        case '=':
            if (next() == '=') {
                kind = TokenKind::EqualsEquals;
                move(2);
            } else if (next() == '>') {
                kind = TokenKind::FatArrow;
                move(2);
            } else {
                kind = TokenKind::Equals;
                move();
            }

            break;

        case '!':
            if (next() == '=') {
                kind = TokenKind::BangEquals;
                move(2);
            } else {
                kind = TokenKind::Bang;
                move();
            }

            break;

        case ':':
            if (next() == ':') {
                kind = TokenKind::Path;
                move(2);
            } else {
                kind = TokenKind::Colon;
                move();
            }

            break;

        case '~':
            kind = TokenKind::Tilde;
            move();
            break;

        case '(':
            kind = TokenKind::SetParen;
            move();
            break;
    
        case ')':
            kind = TokenKind::EndParen;
            move();
            break;

        case '{':
            kind = TokenKind::SetBrace;
            move();
            break;
    
        case '}':
            kind = TokenKind::EndBrace;
            move();
            break;

        case '[':
            kind = TokenKind::SetBrack;
            move();
            break;

        case ']':
            kind = TokenKind::EndBrack;
            move();
            break;

        case '.':
            if (next() == '.') {
                kind = TokenKind::Range;
                move(2);
            } else {
                kind = TokenKind::Dot;
                move();
            }

            break;

        case ',':
            kind = TokenKind::Comma;
            move();
            break;

        case '?':
            kind = TokenKind::Question;
            move();
            break;

        case '@':
            kind = TokenKind::At;
            move();
            break;

        case '#':
            kind = TokenKind::Hash;
            move();
            break;

        case '$':
            kind = TokenKind::Sign;
            move();
            break;

        case ';':
            kind = TokenKind::Semi;
            move();
            break;

        case '\'':
            move();
            kind = TokenKind::Literal;
            literal = LiteralKind::Character;

            if (curr() == '\\') {
                move();
                switch (curr()) {
                    case '0': value = "\0"; break;
                    case 'n': value = "\n"; break;
                    case 't': value = "\t"; break;
                    case 'r': value = "\r"; break;
                    case 'b': value = "\b"; break;
                    case 'f': value = "\f"; break;
                    case 'v': value = "\v"; break;
                    case '\\': value = "\\"; break;
                    case '\'': value = "\'"; break;
                    default: 
                        std::cout << "unknown escape sequence: " << curr() << "\n";
                        exit(1);
                }
            } else
                value = curr();

            if (next() != '\'') {
                kind = TokenKind::Quote;
                literal = LiteralKind::None;
                value = "";
            } else
                move(2);
                
            break;

        case '"':
            move();
            kind = TokenKind::Literal;
            literal = LiteralKind::String;

            while (curr() != '"') {
                if (curr() == '\\') {
                    move();
                    switch (curr()) {
                        case '0': value += '\0'; break;
                        case 'n': value += '\n'; break;
                        case 't': value += '\t'; break;
                        case 'r': value += '\r'; break;
                        case 'b': value += '\b'; break;
                        case 'f': value += '\f'; break;
                        case 'v': value += '\v'; break;
                        case '\\': value += '\\'; break;
                        case '\"': value += '\"'; break;
                        default: 
                            exit(1);
                    }
                } else
                    value += curr();

                move();
            }

            move();
            break;
        
        DEFAULT:
        default: {
            if (isdigit(curr()) || curr() == '-') {
                kind = TokenKind::Literal;
                literal = LiteralKind::Integer;

                if (curr() == '-') {
                    value += curr();
                    move();
                }

                while (isdigit(curr()) || curr() == '.') {
                    if (curr() == '.') {
                        if (!isdigit(next()) || literal == LiteralKind::Float)
                            break;

                        literal = LiteralKind::Float;
                    }
                       
                    value += curr();
                    move();
                }
            } else if (isalpha(curr()) || curr() == '_') {
                kind = TokenKind::Identifier;
                
                while (isalnum(curr()) || curr() == '_') {
                    value += curr();
                    move();
                }
            } else {
                // Handle unknown character
                std::cout << "unknown token: " << curr() << std::endl;
                exit(1);
            }
        }
    };

    return Token(kind, literal, value, start);
}
