#include <algorithm>

#include "scanner.h"

static const char *tokenNames_[] = {
    "end of file",
    "illegal token",
    "identifier",
    "number",
    "'BEGIN'",
    "'END'",
    "'IF'",
    "'THEN'",
    "'ELSE'",
    "'FI'",
    "'WHILE'",
    "'DO'",
    "'OD'",
    "'WRITE'",
    "'READ'",
    "':='",
    "'+' or '-'",
    "'*' or '/'",
    "comparison operator",
    "'('",
    "')'",
    "';'",
    "'&'",
    "'|'",
    "'&&'",
    "'||'",
    "'!'",
    "'true'",
    "'false'",
};

void Scanner::nextToken() {
    skipSpace();

    // Skip the comments.
    while (ch_ == '/') {
        nextChar();
        if (ch_ == '*') {
            nextChar();
            skipSpace();
            bool inside = true;
            while (inside) {
                while (ch_ != '*' && !input_.eof()) {
                    nextChar();
                    skipSpace();
                }

                if (input_.eof()) {
                    token_ = T_EOF;
                    return;
                }

                nextChar();
                if (ch_ == '/') {
                    inside = false;
                    nextChar();
                }
                skipSpace();
            }
        } else {
            token_ = T_MULOP;
            arithmeticValue_ = A_DIVIDE;
            return;
        }

        skipSpace();
    }

    if (input_.eof()) {
        token_ = T_EOF;
        return;
    }

    if (std::isdigit(ch_)) {
        int value = 0;
        while (std::isdigit(ch_)) {
            value = value * 10 + (ch_ - '0');
            nextChar();
        }
        token_ = T_NUMBER;
        intValue_ = value;
    } else if (isIdentifierStart(ch_)) {
        std::string buffer;
        while (isIdentifierBody(ch_)) {
            buffer += ch_;
            nextChar();
        }

        std::transform(buffer.begin(), buffer.end(), buffer.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        std::map<std::string, Token>::iterator kwd = keywords_.find(buffer);
        if (kwd == keywords_.end()) {
            token_ = T_IDENTIFIER;
            stringValue_ = buffer;
        } else {
            token_ = kwd->second;
        }
    } else {
        switch (ch_) {
        case '(':
            token_ = T_LPAREN;
            nextChar();
            break;
        case ')':
            token_ = T_RPAREN;
            nextChar();
            break;
        case ';':
            token_ = T_SEMICOLON;
            nextChar();
            break;
        case ':':
            nextChar();
            if (ch_ == '=') {
                token_ = T_ASSIGN;
                nextChar();

            } else {
                token_ = T_ILLEGAL;
            }
            break;
        case '<':
            token_ = T_CMP;
            nextChar();
            if (ch_ == '=') {
                cmpValue_ = C_LE;
                nextChar();
            } else {
                cmpValue_ = C_LT;
            }
            break;
        case '>':
            token_ = T_CMP;
            nextChar();
            if (ch_ == '=') {
                cmpValue_ = C_GE;
                nextChar();
            } else {
                cmpValue_ = C_GT;
            }
            break;
        case '!':
            nextChar();
            if (ch_ == '=') {
                nextChar();
                token_ = T_CMP;
                cmpValue_ = C_NE;
            } else {
                token_ = T_NOT;
            }
            break;
        case '=':
            token_ = T_CMP;
            cmpValue_ = C_EQ;
            nextChar();
            break;
        case '+':
            token_ = T_ADDOP;
            arithmeticValue_ = A_PLUS;
            nextChar();
            break;

        case '-':
            token_ = T_ADDOP;
            arithmeticValue_ = A_MINUS;
            nextChar();
            break;

        case '*':
            token_ = T_MULOP;
            arithmeticValue_ = A_MULTIPLY;
            nextChar();
            break;
        case '&':
            nextChar();
            if (ch_ == '&') {
                nextChar();
                token_ = T_AND;
            } else {
                token_ = T_LAND;
            }
            break;
        case '|':
            nextChar();
            if (ch_ == '|') {
                nextChar();
                token_ = T_OR;
            } else {
                token_ = T_LOR;
            }
            break;
        default:
            token_ = T_ILLEGAL;
            nextChar();
            break;
        }
    }
}

void Scanner::skipSpace() {
    while (std::isspace(ch_)) {
        if (ch_ == '\n') {
            ++lineNumber_;
        }
        nextChar();
    }
}

void Scanner::nextChar() { ch_ = input_.get(); }

const char *tokenToString(Token t) { return tokenNames_[t]; }
