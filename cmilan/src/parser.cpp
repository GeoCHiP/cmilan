#include <sstream>

#include "parser.h"

Parser::Parser(const std::string &fileName, std::istream &input)
    : m_OutputStream(std::cout), m_Scanner(fileName, input),
      m_Codegen(m_OutputStream) {
    Next();
}

bool Parser::See(Token t) {
    return m_Scanner.GetCurrentToken() == t;
}

bool Parser::Match(Token t) {
    if (m_Scanner.GetCurrentToken() == t) {
        m_Scanner.ExtractNextToken();
        return true;
    } else {
        return false;
    }
}

void Parser::Next() {
    m_Scanner.ExtractNextToken();
}

void Parser::ReportError(const std::string &message) {
    std::cerr << "Line " << m_Scanner.GetLineNumber() << ": " << message
              << std::endl;
    m_IsError = true;
}

void Parser::Parse() {
    Program();
    if (!m_IsError) {
        m_Codegen.flush();
    }
}

void Parser::Program() {
    MustBe(Token::Begin);
    StatementList();
    MustBe(Token::End);
    m_Codegen.emit(STOP);
}

void Parser::StatementList() {
    // If the list of operators is empty, the next token will be one of the
    // possible "closing brackets": END, OD, ELSE, FI. In this case, the result
    // of parsing will be an empty block (its list of operators is null). If
    // the next token is not included in this list, then we consider it the
    // beginning of the operator and we call the statement method. The sign of
    // the last operator is the absence of a semicolon after the operator.
    if (See(Token::End) || See(Token::Od) || See(Token::Else) ||
        See(Token::Fi)) {
        return;
    } else {
        bool more = true;
        while (more) {
            Statement();
            more = Match(Token::Semicolon);
        }
    }
}

void Parser::Statement() {
    if (See(Token::Identifier)) {
        // If we meet a variable, then we remember its address or add a new one
        // if we haven't met it. The next token should be assignment. Then
        // comes the expression block, which returns the value to the top of
        // the stack. We write this value to the address of our variable.

        int varAddress = FindOrAddVariable(m_Scanner.GetStringValue());
        Next();
        MustBe(Token::Assign);
        Expression();
        m_Codegen.emit(STORE, varAddress);
    } else if (Match(Token::If)) {
        // If an IF is encountered, then the condition must follow. There is a
        // 1 or 0 at the top of the stack, depending on the condition being
        // met. Then reserve a place for the conditional JUMP_NO transition to
        // the ELSE block (transition in case of a false condition). The
        // transition address will become known only after the code for the
        // THEN block is generated.

        Relation();

        int jumpNoAddress = m_Codegen.reserve();

        MustBe(Token::Then);
        StatementList();
        if (Match(Token::Else)) {
            // If there is an ELSE block, then in order not to execute it if
            // THEN is executed, we reserve a place for the JUMP command at the
            // end of this block.
            int jumpAddress = m_Codegen.reserve();

            // Fill in the reserved space after checking the condition with the
            // instruction to go to the beginning of the ELSE block.
            m_Codegen.emitAt(jumpNoAddress, JUMP_NO,
                             m_Codegen.getCurrentAddress());

            StatementList();

            // Fill in the second address with an instruction to jump to the
            // end of the conditional ELSE block.
            m_Codegen.emitAt(jumpAddress, JUMP, m_Codegen.getCurrentAddress());
        } else {
            // If there is no ELSE block, then after checking the condition,
            // the following information will be written to the reserved
            // address conditional jump instruction to the end of the IF...THEN
            // statement
            m_Codegen.emitAt(jumpNoAddress, JUMP_NO,
                             m_Codegen.getCurrentAddress());
        }
        MustBe(Token::Fi);
    } else if (Match(Token::While)) {
        // Save the address of the start of the condition check.
        int conditionAddress = m_Codegen.getCurrentAddress();

        Relation();

        // Reserve a place for the conditional jump instruction to exit loop.
        int jumpNoAddress = m_Codegen.reserve();

        MustBe(Token::Do);
        StatementList();
        MustBe(Token::Od);

        // Jump to the address of the loop condition.
        m_Codegen.emit(JUMP, conditionAddress);

        // Fill in the reserved address with the conditional jump instruction
        // to the operator following the loop.
        m_Codegen.emitAt(jumpNoAddress, JUMP_NO,
                         m_Codegen.getCurrentAddress());
    } else if (Match(Token::Write)) {
        MustBe(Token::LeftParen);
        Expression();
        MustBe(Token::RightParen);
        m_Codegen.emit(PRINT);
    } else {
        ReportError("statement expected.");
    }
}

/*
 * An arithmetic expression is described by the following rules:
 * 	<expression> -> <term> | <term> + <term> | <term> - <term>
 *
 * When parsing, we first look at the first term, then analyze the next symbol.
 * If it is '+' or '-', we remove it from the stream and analyze the next term.
 * We repeat this until we encounter a character other than '+' or '-'
 * following the term.
 */
void Parser::Expression() {
    Term();
    while (See(Token::AddOp)) {
        Arithmetic op = m_Scanner.GetArithmeticValue();
        Next();
        Term();

        if (op == Arithmetic::Plus) {
            m_Codegen.emit(ADD);
        } else {
            m_Codegen.emit(SUB);
        }
    }
}

/*
 * The term is described by the following rules:
 *	<expression> -> <factor> | <factor> + <factor> | <factor> - <factor>
 *
 * When parsing, we first look at the first factor, then analyze the next
 * symbol. If it is '*' or '/', remove it from the stream and parse the next
 * factor. We repeat checking and parsing the next factor until we find a
 * symbol other than '*' and '/' following it.
 */
void Parser::Term() {
    Factor();
    while (See(Token::MulOp)) {
        Arithmetic op = m_Scanner.GetArithmeticValue();
        Next();
        Factor();

        if (op == Arithmetic::Multiply) {
            m_Codegen.emit(MULT);
        } else {
            m_Codegen.emit(DIV);
        }
    }
}

/*
 * Factor is described by the following rules:
 *  <factor> -> number | identifier | -<factor> | (<expression>) | READ
 */
void Parser::Factor() {
    if (See(Token::Number)) {
        int value = m_Scanner.GetIntValue();
        Next();
        m_Codegen.emit(PUSH, value);
    } else if (See(Token::Identifier)) {
        int varAddress = FindOrAddVariable(m_Scanner.GetStringValue());
        Next();
        m_Codegen.emit(LOAD, varAddress);
    } else if (See(Token::AddOp) &&
               m_Scanner.GetArithmeticValue() == Arithmetic::Minus) {
        Next();
        Factor();
        m_Codegen.emit(INVERT);
    } else if (Match(Token::LeftParen)) {
        Expression();
        MustBe(Token::RightParen);
    } else if (Match(Token::Read)) {
        m_Codegen.emit(INPUT);
    } else {
        ReportError("expression expected.");
    }
}

// The condition compares two expressions.
// Depending on the result of the comparison
// at the top of the stack it will be 0 or 1.
void Parser::Relation() {
    Expression();
    if (See(Token::Cmp)) {
        Comparison cmp = m_Scanner.GetCmpValue();
        Next();
        Expression();
        switch (cmp) {
        case Comparison::Equal:
            m_Codegen.emit(COMPARE, 0);
            break;
        case Comparison::NotEqual:
            m_Codegen.emit(COMPARE, 1);
            break;
        case Comparison::LessThan:
            m_Codegen.emit(COMPARE, 2);
            break;
        case Comparison::GreaterThan:
            m_Codegen.emit(COMPARE, 3);
            break;
        case Comparison::LessThanOrEqual:
            m_Codegen.emit(COMPARE, 4);
            break;
        case Comparison::GreaterThanOrEqual:
            m_Codegen.emit(COMPARE, 5);
            break;
        };
    } else {
        ReportError("comparison operator expected.");
    }
}

int Parser::FindOrAddVariable(const std::string &var) {
    VarTable::iterator it = m_Variables.find(var);
    if (it == m_Variables.end()) {
        m_Variables[var] = m_LastVariable;
        return m_LastVariable++;
    } else {
        return it->second;
    }
}

void Parser::MustBe(Token t) {
    if (!Match(t)) {
        m_IsError = true;

        std::ostringstream msg;
        msg << TokenToString(m_Scanner.GetCurrentToken()) << " found while "
            << TokenToString(t) << " expected.";
        ReportError(msg.str());

        Recover(t);
    }
}

void Parser::Recover(Token t) {
    while (!See(t) && !See(Token::Eof)) {
        Next();
    }

    if (See(t)) {
        Next();
    }
}
