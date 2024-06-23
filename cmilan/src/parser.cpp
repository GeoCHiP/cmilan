#include <sstream>

#include "parser.h"

Parser::Parser(const std::string &fileName, std::istream &input)
    : m_Scanner(fileName, input), m_OutputStream(std::cout),
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
    MustBe(T_BEGIN);
    StatementList();
    MustBe(T_END);
    m_Codegen.emit(STOP);
}

void Parser::StatementList() {
    // If the list of operators is empty, the next token will be one of the
    // possible "closing brackets": END, OD, ELSE, FI. In this case, the result
    // of parsing will be an empty block (its list of operators is null). If
    // the next token is not included in this list, then we consider it the
    // beginning of the operator and we call the statement method. The sign of
    // the last operator is the absence of a semicolon after the operator.
    if (See(T_END) || See(T_OD) || See(T_ELSE) || See(T_FI)) {
        return;
    } else {
        bool more = true;
        while (more) {
            Statement();
            more = Match(T_SEMICOLON);
        }
    }
}

void Parser::Statement() {
    if (See(T_IDENTIFIER)) {
        // If we meet a variable, then we remember its address or add a new one
        // if we haven't met it. The next token should be assignment. Then
        // comes the expression block, which returns the value to the top of
        // the stack. We write this value to the address of our variable.

        int varAddress = FindOrAddVariable(m_Scanner.GetStringValue());
        Next();
        MustBe(T_ASSIGN);
        LogicalOrExpression();
        m_Codegen.emit(STORE, varAddress);
    } else if (Match(T_IF)) {
        // If an IF is encountered, then the condition must follow. There is a
        // 1 or 0 at the top of the stack, depending on the condition being
        // met. Then reserve a place for the conditional JUMP_NO transition to
        // the ELSE block (transition in case of a false condition). The
        // transition address will become known only after the code for the
        // THEN block is generated.

        LogicalOrExpression();

        int jumpNoAddress = m_Codegen.reserve();

        MustBe(T_THEN);
        StatementList();
        if (Match(T_ELSE)) {
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
        MustBe(T_FI);
    } else if (Match(T_WHILE)) {
        // Save the address of the start of the condition check.
        int conditionAddress = m_Codegen.getCurrentAddress();

        LogicalOrExpression();

        // Reserve a place for the conditional jump instruction to exit loop.
        int jumpNoAddress = m_Codegen.reserve();

        MustBe(T_DO);
        StatementList();
        MustBe(T_OD);

        // Jump to the address of the loop condition.
        m_Codegen.emit(JUMP, conditionAddress);

        // Fill in the reserved address with the conditional jump instruction
        // to the operator following the loop.
        m_Codegen.emitAt(jumpNoAddress, JUMP_NO,
                         m_Codegen.getCurrentAddress());
    } else if (Match(T_WRITE)) {
        MustBe(T_LPAREN);
        Expression();
        MustBe(T_RPAREN);
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
    while (See(T_ADDOP)) {
        Arithmetic op = m_Scanner.GetArithmeticValue();
        Next();
        Term();

        if (op == A_PLUS) {
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
    while (See(T_MULOP)) {
        Arithmetic op = m_Scanner.GetArithmeticValue();
        Next();
        Factor();

        if (op == A_MULTIPLY) {
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
    if (See(T_NUMBER)) {
        int value = m_Scanner.GetIntValue();
        Next();
        m_Codegen.emit(PUSH, value);
    } else if (See(T_TRUE)) {
        Next();
        m_Codegen.emit(PUSH, 1);
    } else if (See(T_FALSE)) {
        Next();
        m_Codegen.emit(PUSH, 0);
    } else if (See(T_IDENTIFIER)) {
        int varAddress = FindOrAddVariable(m_Scanner.GetStringValue());
        Next();
        m_Codegen.emit(LOAD, varAddress);
    } else if (See(T_ADDOP) && m_Scanner.GetArithmeticValue() == A_MINUS) {
        Next();
        Factor();
        m_Codegen.emit(INVERT);
    } else if (Match(T_LPAREN)) {
        LogicalOrExpression();
        MustBe(T_RPAREN);
    } else if (Match(T_NOT)) {
        Factor();
        m_Codegen.emit(PUSH, 0);
        m_Codegen.emit(COMPARE, 0);
    } else if (Match(T_READ)) {
        m_Codegen.emit(INPUT);
    } else {
        ReportError("expression expected.");
    }
}

void Parser::LogicalAndExpression() {
    Relation();
    while (See(T_LAND)) {
        Next();
        Relation();
        m_Codegen.emit(MULT);
    }

    std::vector<int> jumpFalseAddresses;
    bool has_and = false;
    while (See(T_AND)) {
        has_and = true;
        Next();

        m_Codegen.emit(PUSH, 0);
        m_Codegen.emit(COMPARE, 0);
        jumpFalseAddresses.push_back(m_Codegen.reserve());

        Relation();
    }

    if (has_and) {
        m_Codegen.emit(PUSH, 0);
        m_Codegen.emit(COMPARE, 0);
        jumpFalseAddresses.push_back(m_Codegen.reserve());

        m_Codegen.emit(PUSH, 1);
        int jumpTrueAddress = m_Codegen.reserve();

        for (int jumpFalseAddress : jumpFalseAddresses) {
            m_Codegen.emitAt(jumpFalseAddress, JUMP_YES,
                             m_Codegen.getCurrentAddress());
        }

        m_Codegen.emit(PUSH, 0);
        m_Codegen.emitAt(jumpTrueAddress, JUMP, m_Codegen.getCurrentAddress());
    }
}

void Parser::LogicalOrExpression() {
    LogicalAndExpression();
    while (See(T_LOR)) {
        Next();
        LogicalAndExpression();
        m_Codegen.emit(ADD);
        m_Codegen.emit(PUSH, 0);
        m_Codegen.emit(COMPARE, 3);
    }

    std::vector<int> jumpTrueAddresses;
    bool has_or = false;
    while (See(T_OR)) {
        has_or = true;
        Next();

        m_Codegen.emit(PUSH, 1);
        m_Codegen.emit(COMPARE, 0);
        jumpTrueAddresses.push_back(m_Codegen.reserve());

        Relation();
    }

    if (has_or) {
        m_Codegen.emit(PUSH, 1);
        m_Codegen.emit(COMPARE, 0);
        jumpTrueAddresses.push_back(m_Codegen.reserve());

        m_Codegen.emit(PUSH, 0);
        int jumpFalseAddress = m_Codegen.reserve();

        for (int jumpFalseAddress : jumpTrueAddresses) {
            m_Codegen.emitAt(jumpFalseAddress, JUMP_YES,
                             m_Codegen.getCurrentAddress());
        }

        m_Codegen.emit(PUSH, 1);
        m_Codegen.emitAt(jumpFalseAddress, JUMP,
                         m_Codegen.getCurrentAddress());
    }
}

// The condition compares two expressions.
// Depending on the result of the comparison
// at the top of the stack it will be 0 or 1.
void Parser::Relation() {
    Expression();
    if (See(T_CMP)) {
        Cmp cmp = m_Scanner.GetCmpValue();
        Next();
        Expression();
        switch (cmp) {
        case C_EQ:
            m_Codegen.emit(COMPARE, 0);
            break;
        case C_NE:
            m_Codegen.emit(COMPARE, 1);
            break;
        case C_LT:
            m_Codegen.emit(COMPARE, 2);
            break;
        case C_GT:
            m_Codegen.emit(COMPARE, 3);
            break;
        case C_LE:
            m_Codegen.emit(COMPARE, 4);
            break;
        case C_GE:
            m_Codegen.emit(COMPARE, 5);
            break;
        };
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
    while (!See(t) && !See(T_EOF)) {
        Next();
    }

    if (See(t)) {
        Next();
    }
}
