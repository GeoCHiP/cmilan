#include <algorithm>
#include <cctype>

#include "scanner.h"

static const char *s_TokenNames[] = {
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

Scanner::Scanner(const std::string &fileName, std::istream &input)
    : m_FileName(fileName), m_InputStream(input) {
    m_Keywords["begin"] = T_BEGIN;
    m_Keywords["end"] = T_END;
    m_Keywords["if"] = T_IF;
    m_Keywords["then"] = T_THEN;
    m_Keywords["else"] = T_ELSE;
    m_Keywords["fi"] = T_FI;
    m_Keywords["while"] = T_WHILE;
    m_Keywords["do"] = T_DO;
    m_Keywords["od"] = T_OD;
    m_Keywords["write"] = T_WRITE;
    m_Keywords["read"] = T_READ;
    m_Keywords["true"] = T_TRUE;
    m_Keywords["false"] = T_FALSE;

    ExtractNextChar();
}

const std::string &Scanner::GetFileName() const {
    return m_FileName;
}

int Scanner::GetLineNumber() const {
    return m_LineNumber;
}

Token Scanner::GetCurrentToken() const {
    return m_CurrentToken;
}

int Scanner::GetIntValue() const {
    return m_IntValue;
}

std::string Scanner::GetStringValue() const {
    return m_StringValue;
}

Cmp Scanner::GetCmpValue() const {
    return m_CmpValue;
}

Arithmetic Scanner::GetArithmeticValue() const {
    return m_ArithmeticValue;
}

static bool IsIdentifierStart(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static bool IsIdentifierBody(char c) {
    return IsIdentifierStart(c) || std::isdigit(c);
}

void Scanner::ExtractNextToken() {
    SkipSpace();

    // Skip the comments.
    while (m_CurrentChar == '/') {
        ExtractNextChar();
        if (m_CurrentChar == '*') {
            ExtractNextChar();
            SkipSpace();
            bool inside = true;
            while (inside) {
                while (m_CurrentChar != '*' && !m_InputStream.eof()) {
                    ExtractNextChar();
                    SkipSpace();
                }

                if (m_InputStream.eof()) {
                    m_CurrentToken = T_EOF;
                    return;
                }

                ExtractNextChar();
                if (m_CurrentChar == '/') {
                    inside = false;
                    ExtractNextChar();
                }
                SkipSpace();
            }
        } else {
            m_CurrentToken = T_MULOP;
            m_ArithmeticValue = A_DIVIDE;
            return;
        }

        SkipSpace();
    }

    if (m_InputStream.eof()) {
        m_CurrentToken = T_EOF;
        return;
    }

    if (std::isdigit(m_CurrentChar)) {
        int value = 0;
        while (std::isdigit(m_CurrentChar)) {
            value = value * 10 + (m_CurrentChar - '0');
            ExtractNextChar();
        }
        m_CurrentToken = T_NUMBER;
        m_IntValue = value;
    } else if (IsIdentifierStart(m_CurrentChar)) {
        std::string buffer;
        while (IsIdentifierBody(m_CurrentChar)) {
            buffer += m_CurrentChar;
            ExtractNextChar();
        }

        std::transform(buffer.begin(), buffer.end(), buffer.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        std::map<std::string, Token>::iterator kwd = m_Keywords.find(buffer);
        if (kwd == m_Keywords.end()) {
            m_CurrentToken = T_IDENTIFIER;
            m_StringValue = buffer;
        } else {
            m_CurrentToken = kwd->second;
        }
    } else {
        switch (m_CurrentChar) {
        case '(':
            m_CurrentToken = T_LPAREN;
            ExtractNextChar();
            break;
        case ')':
            m_CurrentToken = T_RPAREN;
            ExtractNextChar();
            break;
        case ';':
            m_CurrentToken = T_SEMICOLON;
            ExtractNextChar();
            break;
        case ':':
            ExtractNextChar();
            if (m_CurrentChar == '=') {
                m_CurrentToken = T_ASSIGN;
                ExtractNextChar();

            } else {
                m_CurrentToken = T_ILLEGAL;
            }
            break;
        case '<':
            m_CurrentToken = T_CMP;
            ExtractNextChar();
            if (m_CurrentChar == '=') {
                m_CmpValue = C_LE;
                ExtractNextChar();
            } else {
                m_CmpValue = C_LT;
            }
            break;
        case '>':
            m_CurrentToken = T_CMP;
            ExtractNextChar();
            if (m_CurrentChar == '=') {
                m_CmpValue = C_GE;
                ExtractNextChar();
            } else {
                m_CmpValue = C_GT;
            }
            break;
        case '!':
            ExtractNextChar();
            if (m_CurrentChar == '=') {
                ExtractNextChar();
                m_CurrentToken = T_CMP;
                m_CmpValue = C_NE;
            } else {
                m_CurrentToken = T_NOT;
            }
            break;
        case '=':
            m_CurrentToken = T_CMP;
            m_CmpValue = C_EQ;
            ExtractNextChar();
            break;
        case '+':
            m_CurrentToken = T_ADDOP;
            m_ArithmeticValue = A_PLUS;
            ExtractNextChar();
            break;

        case '-':
            m_CurrentToken = T_ADDOP;
            m_ArithmeticValue = A_MINUS;
            ExtractNextChar();
            break;

        case '*':
            m_CurrentToken = T_MULOP;
            m_ArithmeticValue = A_MULTIPLY;
            ExtractNextChar();
            break;
        case '&':
            ExtractNextChar();
            if (m_CurrentChar == '&') {
                ExtractNextChar();
                m_CurrentToken = T_AND;
            } else {
                m_CurrentToken = T_LAND;
            }
            break;
        case '|':
            ExtractNextChar();
            if (m_CurrentChar == '|') {
                ExtractNextChar();
                m_CurrentToken = T_OR;
            } else {
                m_CurrentToken = T_LOR;
            }
            break;
        default:
            m_CurrentToken = T_ILLEGAL;
            ExtractNextChar();
            break;
        }
    }
}

void Scanner::SkipSpace() {
    while (std::isspace(m_CurrentChar)) {
        if (m_CurrentChar == '\n') {
            ++m_LineNumber;
        }
        ExtractNextChar();
    }
}

void Scanner::ExtractNextChar() {
    m_CurrentChar = m_InputStream.get();
}

const char *TokenToString(Token t) {
    return s_TokenNames[t];
}
