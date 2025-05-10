#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

void Parser::backtrack(unsigned n) {
    m_Stream.setPos(m_Stream.getPos() - n);
    m_Current = &m_Stream.m_Buffer[m_Stream.m_Iter - 1];
}

void Parser::skip(unsigned n) {
    m_Stream.setPos(m_Stream.getPos() + n);
    m_Current = &m_Stream.m_Buffer[m_Stream.m_Iter - 1];
}

int Parser::get_bin_precedence() const {
    switch (m_Current->kind) {
    /// *, /, %
    case TokenKind::Star:
    case TokenKind::Slash:
    case TokenKind::Percent:
        return 11;

    /// +, -
    case TokenKind::Plus:
    case TokenKind::Minus:
        return 10;

    /// <<, >>
    case TokenKind::LeftLeft:
    case TokenKind::RightRight:
        return 9;

    /// <, <=, >, >=
    case TokenKind::Left:
    case TokenKind::LeftEquals:
    case TokenKind::Right:
    case TokenKind::RightEquals:
        return 8;

    /// ==, !=
    case TokenKind::EqualsEquals:
    case TokenKind::BangEquals:
        return 7;

    /// &
    case TokenKind::And:
        return 6;

    /// ^
    case TokenKind::Xor:
        return 5;

    /// |
    case TokenKind::Or:
        return 4;

    /// &&
    case TokenKind::AndAnd:
        return 3;

    /// ||
    case TokenKind::OrOr:
        return 2;

    /// Assignment operators, i.e. +, +=, -=, ...
    case TokenKind::Equals:
    case TokenKind::PlusEquals:
    case TokenKind::MinusEquals:
    case TokenKind::StarEquals:
    case TokenKind::SlashEquals:
    case TokenKind::PercentEquals:
    case TokenKind::AndEquals:
    case TokenKind::OrEquals:
    case TokenKind::XorEquals:
    case TokenKind::LeftLeftEquals:
    case TokenKind::RightRightEquals:
        return 1;

    default:
        return -1;
    }
}

BinaryExpr::Kind Parser::get_bin_operator() const {
    switch (m_Current->kind) {
        case TokenKind::Star: return BinaryExpr::Kind::Mul;
        case TokenKind::Slash: return BinaryExpr::Kind::Div;
        case TokenKind::Percent: return BinaryExpr::Kind::Mod;
        case TokenKind::Plus: return BinaryExpr::Kind::Add;
        case TokenKind::Minus: return BinaryExpr::Kind::Sub;
        case TokenKind::LeftLeft: return BinaryExpr::Kind::LeftShift;
        case TokenKind::RightRight: return BinaryExpr::Kind::RightShift;
        case TokenKind::Left: return BinaryExpr::Kind::LessThan;
        case TokenKind::LeftEquals: return BinaryExpr::Kind::LessThanEquals;
        case TokenKind::Right: return BinaryExpr::Kind::GreaterThan;
        case TokenKind::RightEquals: return BinaryExpr::Kind::GreaterThanEquals;
        case TokenKind::EqualsEquals: return BinaryExpr::Kind::Equals;
        case TokenKind::BangEquals: return BinaryExpr::Kind::NEquals;
        case TokenKind::And: return BinaryExpr::Kind::Bitwise_And;
        case TokenKind::Xor: return BinaryExpr::Kind::Bitwise_Xor;
        case TokenKind::Or: return BinaryExpr::Kind::Bitwise_Or;
        case TokenKind::AndAnd: return BinaryExpr::Kind::Logic_And;
        case TokenKind::OrOr: return BinaryExpr::Kind::Logic_Or;
        case TokenKind::Equals: return BinaryExpr::Kind::Assign;
        case TokenKind::PlusEquals: return BinaryExpr::Kind::Add_Assign;
        case TokenKind::MinusEquals: return BinaryExpr::Kind::Sub_Assign;
        case TokenKind::StarEquals: return BinaryExpr::Kind::Mul_Assign;
        case TokenKind::SlashEquals: return BinaryExpr::Kind::Div_Assign;
        case TokenKind::PercentEquals: return BinaryExpr::Kind::Mod_Assign;
        case TokenKind::AndEquals: return BinaryExpr::Kind::And_Assign;
        case TokenKind::OrEquals: return BinaryExpr::Kind::Or_Assign;
        case TokenKind::XorEquals: return BinaryExpr::Kind::Xor_Assign;
        case TokenKind::LeftLeftEquals: return BinaryExpr::Kind::LeftShift_Assign;
        case TokenKind::RightRightEquals: return BinaryExpr::Kind::RightShift_Assign;
        default: return BinaryExpr::Kind::Unknown;
    }
}

UnaryExpr::Kind Parser::get_un_operator() const {
    switch (m_Current->kind) {
        case TokenKind::Minus: return UnaryExpr::Kind::Negate;
        case TokenKind::Tilde: return UnaryExpr::Kind::Bitwise_Not;
        case TokenKind::Bang: return UnaryExpr::Kind::Logic_Not;
        case TokenKind::PlusPlus: return UnaryExpr::Kind::Increment;
        case TokenKind::MinusMinus: return UnaryExpr::Kind::Decrement;
        case TokenKind::And: return UnaryExpr::Kind::Address_Of;
        case TokenKind::Star: return UnaryExpr::Kind::Dereference;
        default: return UnaryExpr::Kind::Unknown;
    }
}

String Parser::parse_type_name() {
    if (!match(TokenKind::Identifier))
        fatal("expected type identifier", &m_Current->md);
    
    Metadata md = m_Current->md;
    String name = m_Current->value;
    next();

    if (match(TokenKind::Path)) {
        NamedDecl *named = m_Scope->lookup(name);
        if (!dynamic_cast<UseDecl *>(named))
            fatal("expected use declaration before namespaced type", &md);

        name += "::";
        next();

        if (!match(TokenKind::Identifier))
            fatal("expected type identifier", &m_Current->md);
        
        name += m_Current->value;
        next();
    }

    if (match(TokenKind::Left)) {
        name += "<";
        next();

        while (!match(TokenKind::Right)) {
            name += parse_type_name();

            if (match(TokenKind::Right))
                break;

            if (!match(TokenKind::Comma))
                fatal("expected ',' or '>' in type argument list", &m_Current->md);
            next();
            name += ", ";
        }

        next(); // Consume '>'
        name += ">";
    }

    while (1) {
        if (match(TokenKind::Star))
            name += '*';
        else if (match(TokenKind::SetBrack)) {
            name += '[';
            next();

            if (!match(LiteralKind::Integer))
                fatal("expected integer literal", &m_Current->md);

            name += m_Current->value;
            next();

            if (!match(TokenKind::EndBrack))
                fatal("expected ']' after array size", &m_Current->md);

            name += ']';
        } else
            break;

        next();
    }

    return name;
}

Type *Parser::parse_type() {
    if (!match(TokenKind::Identifier))
        fatal("expected type identifier", &m_Current->md);
    
    Metadata md = m_Current->md;
    String name = m_Current->value;
    next();

    if (match(TokenKind::Path)) {
        NamedDecl *named = m_Scope->lookup(name);
        if (dynamic_cast<UseDecl *>(named)) {
            name += "::";
            next();

            if (!match(TokenKind::Identifier))
                fatal("expected type identifier", &m_Current->md);
            
            name += m_Current->value;
            next();
        }
    }

    if (match(TokenKind::Left)) {
        name += "<";
        next();

        while (!match(TokenKind::Right)) {
            Type *argTy = parse_type();
            name += argTy->getName();

            if (match(TokenKind::Right))
                break;

            if (!match(TokenKind::Comma))
                fatal("expected ',' or '>' in type argument list", &m_Current->md);
            next();
            name += ", ";
        }

        next(); // Consume '>'
        name += ">";
    }

    while (1) {
        if (match(TokenKind::Star))
            name += '*';
        else if (match(TokenKind::SetBrack)) {
            name += '[';
            next();

            if (!match(LiteralKind::Integer))
                fatal("expected integer literal", &m_Current->md);

            name += m_Current->value;
            next();

            if (!match(TokenKind::EndBrack))
                fatal("expected ']' after array size", &m_Current->md);

            name += ']';
        } else
            break;

        next();
    }

    return Type::get(m_Context, name, m_Scope, m_Current->md);
}

void Parser::parse_runes() {
    if (!match(TokenKind::Sign))
        return;

    next(); // '$'

    bool listed = false;
    if (match(TokenKind::SetBrack)) {
        listed = true;
        next();
    }

    while (1) {
        if (!match(TokenKind::Identifier))
            fatal("expected rune identifier", &m_Current->md);

        String name = m_Current->value;
        if (name == "associated")
            m_Runes.set(Rune::Associated);
        else if (name == "no_mangle")
            m_Runes.set(Rune::NoMangle);
        else if (name == "public")
            m_Runes.set(Rune::Public);
        else
            warn("unknown rune: " + name, &m_Current->md);

        next();

        if (listed) {
            if (match(TokenKind::EndBrack)) {
                next();
                break;
            }

            if (!match(TokenKind::Comma))
                fatal("expected ']' after rune list", &m_Current->md);

            next();
        } else
            break;
    }
}

Parser::Parser(const File &F, const TokenStream &S) : m_Stream(S) {
    // Get the filename with no extension.
    String id = F.filename;
    size_t pos = id.find_last_of('.');
    if (pos != String::npos)
        id = id.substr(0, pos);

    m_Unit = new TranslationUnit(id, F);
    m_Context = m_Unit->getContext();
    m_Scope = m_Unit->getScope();
    m_Current = m_Stream.get();

    while (!m_Stream.isEnd() && !match(TokenKind::Eof)) {
        Decl *D = parse_decl();
        if (!D)
            fatal("expected declaration", &m_Current->md);

        if (auto *use = dynamic_cast<UseDecl *>(D))
            m_Unit->addUse(use);
        else
            m_Unit->addDecl(D);
    }
}
