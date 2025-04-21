#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Type *Parser::parseType(bool produce) {
    if (!match(TokenKind::Identifier))
        fatal("expected type identifier", &m_Current->md);
    
    Metadata md = m_Current->md;
    String name = m_Current->value;
    next();

    /*
    if (match(TokenKind::Left)) {
        name += "<";
        next();

        while (!match(TokenKind::RIGHT)) {
            Type *argTy = parseType();
            if (!argTy) {
                trace("expected type argument", &getLoc());
                return nullptr;
            }

            name += argTy->getName();

            if (match(TokenKind::RIGHT))
                break;

            expect(TokenKind::COMMA);
            next();
            name += ", ";
        }

        next(); // Consume '>'
        name += ">";
    }
    */

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

    if (produce)
        return m_Context->produceType(name, m_Current->md);

    return nullptr;
}

void Parser::parseAttributes() {
    if (!match(TokenKind::Sign))
        return;

    next(); // '$'

    bool listed = false;
    if (match(TokenKind::SetBrack)) {
        listed = true;
        next();
    }

    for (;;) {
        if (!match(TokenKind::Identifier))
            fatal("expected attribute identifier", &m_Current->md);

        String name = m_Current->value;
        if (name == "no_mangle")
            m_Attributes.no_mangle = 1;
        else
            warn("unknown attribute: " + name, &m_Current->md);

        next();

        if (listed) {
            if (match(TokenKind::EndBrack)) {
                next();
                break;
            }

            if (!match(TokenKind::Comma))
                fatal("expected ']' after attribute list", &m_Current->md);

            next();
        } else
            break;
    }
}

Parser::Parser(const File &F, const TokenStream &S) : m_Stream(S) {
    m_Unit = new TranslationUnit(F);
    m_Context = m_Unit->getContext();
    m_Scope = m_Unit->getScope();
    m_Current = m_Stream.get();

    while (!m_Stream.isEnd()) {
        Decl *D = parseDecl();
        if (!D)
            fatal("expected declaration", &m_Current->md);

        m_Unit->addDecl(D);
    }
}
