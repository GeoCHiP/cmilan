#ifndef CMILAN_PARSER_H
#define CMILAN_PARSER_H

#include "codegen.h"
#include "scanner.h"

/* Parser.
 *
 * Tasks:
 * - checking the correctness of the program,
 * - generating code for a virtual machine during analysis,
 * - the simplest error recovery.
 *
 * The Milan language parser.
 *
 * Parser using the lexical analyzer passed to it during initialization
 * reads one token at a time and generates code for a stack virtual machine
 * based on Milan grammar. The syntactic analysis is performed by the
 * recursive descent method.
 *
 * When an error is detected, the parser prints a message and continues the
 * analysis with the next operator in order to find as many errors as possible
 * during the parsing process. Since the error recovery strategy is very simple,
 * printing is possible reports of non-existent ("induced") errors or skipping
 * some errors without printing messages. If at least one error was found during
 * the parsing process, the code for the VM is not printed.
 * */

class Parser {
public:
    // The constructor creates instances of the lexical analyzer and code
    // generator.
    Parser(const std::string &fileName, std::istream &input)
        : output_(std::cout), error_(false), lastVar_(0) {
        scanner_ = new Scanner(fileName, input);
        codegen_ = new CodeGen(output_);
        next();
    }

    ~Parser() {
        delete codegen_;
        delete scanner_;
    }

    void parse();

private:
    // Non-terminals
    void program();
    void statementList();
    void statement();
    void expression();
    void term();
    void factor();
    void relation();
    void logicalAndExpression();
    void logicalOrExpression();

    // Comparing the current token with the target. The current position in the
    // token stream does not change.
    bool see(Token t) { return scanner_->token() == t; }

    // Checking the match of the current token with the target. If the token and
    // the target match, the token is removed from the stream.
    bool match(Token t) {
        if (scanner_->token() == t) {
            scanner_->nextToken();
            return true;
        } else {
            return false;
        }
    }

    void next() { scanner_->nextToken(); }

    void reportError(const std::string &message) {
        std::cerr << "Line " << scanner_->getLineNumber() << ": " << message
                  << std::endl;
        error_ = true;
    }

    // Check if this token matches the target. If so, the token is removed from
    // the stream. Otherwise, we create an error message and try to recover.
    void mustBe(Token t);

    // Error recovery: analyze the code until we meet this token or
    // the end-of-file token.
    void recover(Token t);

    // If it finds the desired variable, it returns its number, otherwise it
    // adds the variable to the array, increases lastVar and returns it.
    int findOrAddVariable(const std::string &variableName);

    using VarTable = std::map<std::string, int>;

private:
    Scanner *scanner_;
    CodeGen *codegen_;
    std::ostream &output_;
    bool error_;
    VarTable variables_;
	// the number of the last recorded variable
    int lastVar_;
};

#endif
