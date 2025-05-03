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

inline String kindToString(TokenKind K) {
	switch (K) {
		case TokenKind::Identifier: return "Identifier";
		case TokenKind::Literal: return "Literal";
		case TokenKind::Whitespace: return "Whitespace";
		case TokenKind::Newline: return "Newline";
		case TokenKind::LineComment: return "LineComment";
		case TokenKind::BlockComment: return "BlockComment";
		case TokenKind::Plus: return "Plus";
		case TokenKind::PlusPlus: return "PlusPlus";
		case TokenKind::PlusEquals: return "PlusEquals";
		case TokenKind::Minus: return "Minus";
		case TokenKind::MinusMinus: return "MinusMinus";
		case TokenKind::MinusEquals: return "MinusEquals";
		case TokenKind::Star: return "Star";
		case TokenKind::StarEquals: return "StarEquals";
		case TokenKind::Slash: return "Slash";
		case TokenKind::SlashSlash: return "SlashSlash";
		case TokenKind::SlashEquals: return "SlashEquals";
		case TokenKind::Left: return "Left";
		case TokenKind::LeftLeft: return "LeftLeft";
		case TokenKind::LeftEquals: return "LeftEquals";
		case TokenKind::LeftLeftEquals: return "LeftLeftEquals";
		case TokenKind::Right: return "Right";
		case TokenKind::RightRight: return "RightRight";
		case TokenKind::RightEquals: return "RightEquals";
		case TokenKind::RightRightEquals: return "RightRightEquals";
		case TokenKind::And: return "And";
		case TokenKind::AndAnd: return "AndAnd";
		case TokenKind::AndEquals: return "AndEquals";
		case TokenKind::Or: return "Or";
		case TokenKind::OrOr: return "OrOr";
		case TokenKind::OrEquals: return "OrEquals";
		case TokenKind::Xor: return "Xor";
		case TokenKind::XorEquals: return "XorEquals";
		case TokenKind::Percent: return "Percent";
		case TokenKind::PercentEquals: return "PercentEquals";
		case TokenKind::Equals: return "Equals";
		case TokenKind::EqualsEquals: return "EqualsEquals";
		case TokenKind::Bang: return "Bang";
		case TokenKind::BangEquals: return "BangEquals";
		case TokenKind::Path: return "Path";
		case TokenKind::Tilde: return "Tilde";
		case TokenKind::SetParen: return "SetParen";
		case TokenKind::EndParen: return "EndParen";
		case TokenKind::SetBrace: return "SetBrace";
		case TokenKind::EndBrace: return "EndBrace";
		case TokenKind::SetBrack: return "SetBrack";
		case TokenKind::EndBrack: return "EndBrack";
		case TokenKind::Arrow: return "Arrow";
		case TokenKind::FatArrow: return "FatArrow";
		case TokenKind::Dot: return "Dot";
		case TokenKind::Range: return "Range";
		case TokenKind::LongRange: return "LongRange";
		case TokenKind::Colon: return "Colon";
		case TokenKind::Comma: return "Comma";
		case TokenKind::Quote: return "Quote";
		case TokenKind::Grave: return "Grave";
		case TokenKind::Semi: return "Semi";
		case TokenKind::Question: return "Question";
		case TokenKind::At: return "At";
		case TokenKind::Hash: return "Hash";
		case TokenKind::Sign: return "Sign";
		case TokenKind::Eof: return "Eof";
		default: return "Unknown";
	}
}

} // namespace meddle

#endif // MEDDLE_TOKEN_H
