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
const char *tokenToString(Token t);

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
    explicit Scanner(const std::string &fileName, std::istream &input)
        : fileName_(fileName), lineNumber_(1), input_(input) {
        keywords_["begin"] = T_BEGIN;
        keywords_["end"] = T_END;
        keywords_["if"] = T_IF;
        keywords_["then"] = T_THEN;
        keywords_["else"] = T_ELSE;
        keywords_["fi"] = T_FI;
        keywords_["while"] = T_WHILE;
        keywords_["do"] = T_DO;
        keywords_["od"] = T_OD;
        keywords_["write"] = T_WRITE;
        keywords_["read"] = T_READ;
        keywords_["true"] = T_TRUE;
        keywords_["false"] = T_FALSE;

        nextChar();
    }

    virtual ~Scanner() {}

    const std::string &getFileName() const { return fileName_; }

    int getLineNumber() const { return lineNumber_; }

    Token token() const { return token_; }

    int getIntValue() const { return intValue_; }

    std::string getStringValue() const { return stringValue_; }

    Cmp getCmpValue() const { return cmpValue_; }

    Arithmetic getArithmeticValue() const { return arithmeticValue_; }

    // Exctract the next lexeme.
    // Next lexeme is saved in token_ and extracted from the stream.
    void nextToken();

private:
    // Skip all the whitespace characters.
    // If new-line character found, increment lineNumber_
    void skipSpace();

    void nextChar();

    bool isIdentifierStart(char c) {
        return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
    }

    bool isIdentifierBody(char c) { return isIdentifierStart(c) || isdigit(c); }

private:
    const std::string fileName_;
    int lineNumber_;
    Token token_;
    int intValue_;
    // Variable name
    std::string stringValue_;
    Cmp cmpValue_;
    Arithmetic arithmeticValue_;
    std::map<std::string, Token> keywords_;
    std::istream &input_;
    char ch_;
};

#endif // CMILAN_SCANNER_H
