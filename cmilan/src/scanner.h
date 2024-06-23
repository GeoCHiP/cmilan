#ifndef CMILAN_SCANNER_H
#define CMILAN_SCANNER_H

#include <fstream>
#include <map>

enum Token {
    T_EOF,
    T_ILLEGAL,
    T_IDENTIFIER,
    T_NUMBER,
    T_BEGIN,
    T_END,
    T_IF,
    T_THEN,
    T_ELSE,
    T_FI,
    T_WHILE,
    T_DO,
    T_OD,
    T_WRITE,
    T_READ,
    T_ASSIGN,
    T_ADDOP, // lexeme for "+" and "-"
    T_MULOP, // lexeme for "*" and "/"
    T_CMP,
    T_LPAREN,
    T_RPAREN,
    T_SEMICOLON,
    T_LAND, // lexeme for "&"
    T_LOR,  // lexeme for "|"
    T_AND,  // lexeme for "&&"
    T_OR,   // lexeme for "||"
    T_NOT,
    T_TRUE,  // lexeme for "true"
    T_FALSE, // lexeme for "false"
};

// Returns lexeme description.
const char *TokenToString(Token t);

// Type of comparison operation.
enum Cmp {
    C_EQ,
    C_NE,
    C_LT,
    C_LE,
    C_GT,
    C_GE,
};

// Type of arithmetic operation.
enum Arithmetic {
    A_PLUS,
    A_MINUS,
    A_MULTIPLY,
    A_DIVIDE,
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
    Cmp GetCmpValue() const;
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
    Cmp m_CmpValue;
    Arithmetic m_ArithmeticValue;
    std::map<std::string, Token> m_Keywords;
    std::istream &m_InputStream;
};

#endif // CMILAN_SCANNER_H
