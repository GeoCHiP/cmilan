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
    Parser(const std::string &fileName, std::istream &input);

    void Parse();

private:
    using VarTable = std::map<std::string, int>;

    // Non-terminals
    void Program();
    void StatementList();
    void Statement();
    void Expression();
    void Term();
    void Factor();
    void Relation();
    void LogicalAndExpression();
    void LogicalOrExpression();

    // Comparing the current token with the target. The current position in the
    // token stream does not change.
    bool See(Token t);

    // Checking the match of the current token with the target. If the token and
    // the target match, the token is removed from the stream.
    bool Match(Token t);

    void Next();

    void ReportError(const std::string &message);

    // Check if this token matches the target. If so, the token is removed from
    // the stream. Otherwise, we create an error message and try to recover.
    void MustBe(Token t);

    // Error recovery: analyze the code until we meet this token or
    // the end-of-file token.
    void Recover(Token t);

    // If it finds the desired variable, it returns its number, otherwise it
    // adds the variable to the array, increases lastVar and returns it.
    int FindOrAddVariable(const std::string &variableName);

private:
    std::ostream &m_OutputStream;
    Scanner m_Scanner;
    CodeGen m_Codegen;
    VarTable m_Variables;
    bool m_IsError = false;
	// the number of the last recorded variable
    int m_LastVariable = 0;
};

#endif
