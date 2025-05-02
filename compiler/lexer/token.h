#ifndef MEDDLE_TOKEN_H
#define MEDDLE_TOKEN_H

#include "../core/metadata.h"

#include <string>

using String = std::string;

namespace meddle {

/// The different kinds of tokens.
enum class TokenKind {
    Identifier,
    Literal,

    Whitespace,
    Newline,
    LineComment,
    BlockComment,

    /// +
    Plus,
    /// ++
    PlusPlus,
    /// +=
    PlusEquals,
    /// -
    Minus,
    /// --
    MinusMinus,
    /// -=
    MinusEquals,
    /// *
    Star,
    /// *=
    StarEquals,
    /// /
    Slash,
    /// //
    SlashSlash,
    /// /=
    SlashEquals,
    /// <
    Left,
    /// <<
    LeftLeft,
    /// <=
    LeftEquals,
    /// <<=
    LeftLeftEquals,
    /// >
    Right,
    /// >>
    RightRight,
    /// >=
    RightEquals,
    /// >>=
    RightRightEquals,
    /// &
    And,
    /// &&
    AndAnd,
    /// &=
    AndEquals,
    /// |
    Or,
    /// ||
    OrOr,
    /// |=
    OrEquals,
    /// ^
    Xor,
    /// ^=
    XorEquals,
    /// %
    Percent,
    /// %=
    PercentEquals,
    /// =
    Equals,
    /// ==
    EqualsEquals,
    /// !
    Bang,
    /// !=
    BangEquals,
    /// ::
    Path,
    /// ~
    Tilde,

    /// (
    SetParen,
    /// )
    EndParen,
    /// {
    SetBrace,
    /// }
    EndBrace,
    /// [
    SetBrack,
    /// ]
    EndBrack,

    /// ->
    Arrow,
    /// =>
    FatArrow,

    /// .
    Dot,
    /// ..
    Range,
    /// ...
    LongRange,
    /// :
    Colon,
    /// ,
    Comma,
    /// '
    Quote,
    /// `
    Grave,
    /// ;
    Semi,

    /// ?
    Question,
    /// @
    At,
    /// #
    Hash,
    /// $
    Sign,

    /// End of file.
    Eof,
};

/// The different kinds of literal tokens.
enum class LiteralKind {
    None = 0,
    Character,
    String,
    Integer,
    Float,
};

struct Token final {
    TokenKind kind;
    LiteralKind literal = LiteralKind::None;
    String value;
    Metadata md;

    Token(const Metadata &md) : kind(TokenKind::Eof), md(md) {}

    Token(TokenKind kind, const String &value, const Metadata &md)
      : kind(kind), value(value), md(md) {}

    Token(
      TokenKind kind, 
      LiteralKind literal, 
      const String &value,
      const Metadata &md
    ) : kind(kind), literal(literal), value(value), md(md) {}
};

} // namespace meddle

#endif // MEDDLE_TOKEN_H
