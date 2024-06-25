#ifndef CMILAN_SCANNER_H
#define CMILAN_SCANNER_H

#include <istream>
#include <map>
#include <string>

enum class Token {
    Eof,
    Illegal,
    Identifier,
    Number,
    Begin,
    End,
    If,
    Then,
    Else,
    Fi,
    While,
    Do,
    Od,
    Write,
    Read,
    Assign,
    AddOp, // lexeme for "+" and "-"
    MulOp, // lexeme for "*" and "/"
    Cmp,
    LeftParen,
    RightParen,
    Semicolon,
};

// Returns lexeme description.
const char *TokenToString(Token t);

// Type of comparison operation.
enum class Comparison {
    Equal,
    NotEqual,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,
};

// Type of arithmetic operation.
enum class Arithmetic {
    Plus,
    Minus,
    Multiply,
    Divide,
};

// Lexial analyzer.
class Scanner {
public:
    explicit Scanner(const std::string &fileName, std::istream &input);

    const std::string &GetFileName() const;
    int GetLineNumber() const;
    Token GetCurrentToken() const;
    int GetIntValue() const;
    std::string GetStringValue() const;
    Comparison GetCmpValue() const;
    Arithmetic GetArithmeticValue() const;

    // Exctract the next lexeme.
    // Next lexeme is saved in token_ and extracted from the stream.
    void ExtractNextToken();

private:
    void ExtractNextChar();

    // Skip all the whitespace characters.
    // If new-line character found, increment lineNumber_
    void SkipSpace();

private:
    const std::string m_FileName;
    int m_LineNumber = 1;
    char m_CurrentChar;
    Token m_CurrentToken;
    int m_IntValue = 0;
    // Variable name
    std::string m_StringValue;
    Comparison m_CmpValue;
    Arithmetic m_ArithmeticValue;
    std::map<std::string, Token> m_Keywords;
    std::istream &m_InputStream;
};

#endif // CMILAN_SCANNER_H
