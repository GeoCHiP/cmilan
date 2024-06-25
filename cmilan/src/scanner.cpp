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
};

Scanner::Scanner(const std::string &fileName, std::istream &input)
    : m_FileName(fileName), m_InputStream(input) {
    m_Keywords["begin"] = Token::Begin;
    m_Keywords["end"] = Token::End;
    m_Keywords["if"] = Token::If;
    m_Keywords["then"] = Token::Then;
    m_Keywords["else"] = Token::Else;
    m_Keywords["fi"] = Token::Fi;
    m_Keywords["while"] = Token::While;
    m_Keywords["do"] = Token::Do;
    m_Keywords["od"] = Token::Od;
    m_Keywords["write"] = Token::Write;
    m_Keywords["read"] = Token::Read;

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

Comparison Scanner::GetCmpValue() const {
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
                    m_CurrentToken = Token::Eof;
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
            m_CurrentToken = Token::MulOp;
            m_ArithmeticValue = Arithmetic::Divide;
            return;
        }

        SkipSpace();
    }

    if (m_InputStream.eof()) {
        m_CurrentToken = Token::Eof;
        return;
    }

    if (std::isdigit(m_CurrentChar)) {
        int value = 0;
        while (std::isdigit(m_CurrentChar)) {
            value = value * 10 + (m_CurrentChar - '0');
            ExtractNextChar();
        }
        m_CurrentToken = Token::Number;
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
            m_CurrentToken = Token::Identifier;
            m_StringValue = buffer;
        } else {
            m_CurrentToken = kwd->second;
        }
    } else {
        switch (m_CurrentChar) {
        case '(':
            m_CurrentToken = Token::LeftParen;
            ExtractNextChar();
            break;
        case ')':
            m_CurrentToken = Token::RightParen;
            ExtractNextChar();
            break;
        case ';':
            m_CurrentToken = Token::Semicolon;
            ExtractNextChar();
            break;
        case ':':
            ExtractNextChar();
            if (m_CurrentChar == '=') {
                m_CurrentToken = Token::Assign;
                ExtractNextChar();

            } else {
                m_CurrentToken = Token::Illegal;
            }
            break;
        case '<':
            m_CurrentToken = Token::Cmp;
            ExtractNextChar();
            if (m_CurrentChar == '=') {
                m_CmpValue = Comparison::LessThanOrEqual;
                ExtractNextChar();
            } else {
                m_CmpValue = Comparison::LessThan;
            }
            break;
        case '>':
            m_CurrentToken = Token::Cmp;
            ExtractNextChar();
            if (m_CurrentChar == '=') {
                m_CmpValue = Comparison::GreaterThanOrEqual;
                ExtractNextChar();
            } else {
                m_CmpValue = Comparison::GreaterThan;
            }
            break;
        case '!':
            ExtractNextChar();
            if (m_CurrentChar == '=') {
                ExtractNextChar();
                m_CurrentToken = Token::Cmp;
                m_CmpValue = Comparison::NotEqual;
            } else {
                m_CurrentToken = Token::Illegal;
            }
            break;
        case '=':
            m_CurrentToken = Token::Cmp;
            m_CmpValue = Comparison::Equal;
            ExtractNextChar();
            break;
        case '+':
            m_CurrentToken = Token::AddOp;
            m_ArithmeticValue = Arithmetic::Plus;
            ExtractNextChar();
            break;

        case '-':
            m_CurrentToken = Token::AddOp;
            m_ArithmeticValue = Arithmetic::Minus;
            ExtractNextChar();
            break;

        case '*':
            m_CurrentToken = Token::MulOp;
            m_ArithmeticValue = Arithmetic::Multiply;
            ExtractNextChar();
            break;

        default:
            m_CurrentToken = Token::Illegal;
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
    return s_TokenNames[static_cast<int>(t)];
}
