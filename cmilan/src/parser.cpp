#include <sstream>

#include "parser.h"

void Parser::parse() {
    program();
    if (!error_) {
        codegen_->flush();
    }
}

void Parser::program() {
    mustBe(T_BEGIN);
    statementList();
    mustBe(T_END);
    codegen_->emit(STOP);
}

void Parser::statementList() {
    // If the list of operators is empty, the next token will be one of the
    // possible "closing brackets": END, OD, ELSE, FI. In this case, the result
    // of parsing will be an empty block (its list of operators is null). If
    // the next token is not included in this list, then we consider it the
    // beginning of the operator and we call the statement method. The sign of
    // the last operator is the absence of a semicolon after the operator.
    if (see(T_END) || see(T_OD) || see(T_ELSE) || see(T_FI)) {
        return;
    } else {
        bool more = true;
        while (more) {
            statement();
            more = match(T_SEMICOLON);
        }
    }
}

void Parser::statement() {
    if (see(T_IDENTIFIER)) {
        // If we meet a variable, then we remember its address or add a new one
        // if we haven't met it. The next token should be assignment. Then
        // comes the expression block, which returns the value to the top of
        // the stack. We write this value to the address of our variable.

        int varAddress = findOrAddVariable(scanner_->GetStringValue());
        next();
        mustBe(T_ASSIGN);
        logicalOrExpression();
        codegen_->emit(STORE, varAddress);
    } else if (match(T_IF)) {
        // If an IF is encountered, then the condition must follow. There is a
        // 1 or 0 at the top of the stack, depending on the condition being
        // met. Then reserve a place for the conditional JUMP_NO transition to
        // the ELSE block (transition in case of a false condition). The
        // transition address will become known only after the code for the
        // THEN block is generated.

        logicalOrExpression();

        int jumpNoAddress = codegen_->reserve();

        mustBe(T_THEN);
        statementList();
        if (match(T_ELSE)) {
            // If there is an ELSE block, then in order not to execute it if
            // THEN is executed, we reserve a place for the JUMP command at the
            // end of this block.
            int jumpAddress = codegen_->reserve();

            // Fill in the reserved space after checking the condition with the
            // instruction to go to the beginning of the ELSE block.
            codegen_->emitAt(jumpNoAddress, JUMP_NO,
                             codegen_->getCurrentAddress());

            statementList();

            // Fill in the second address with an instruction to jump to the
            // end of the conditional ELSE block.
            codegen_->emitAt(jumpAddress, JUMP, codegen_->getCurrentAddress());
        } else {
            // If there is no ELSE block, then after checking the condition,
            // the following information will be written to the reserved
            // address conditional jump instruction to the end of the IF...THEN
            // statement
            codegen_->emitAt(jumpNoAddress, JUMP_NO,
                             codegen_->getCurrentAddress());
        }
        mustBe(T_FI);
    } else if (match(T_WHILE)) {
        // Save the address of the start of the condition check.
        int conditionAddress = codegen_->getCurrentAddress();

        logicalOrExpression();

        // Reserve a place for the conditional jump instruction to exit loop.
        int jumpNoAddress = codegen_->reserve();

        mustBe(T_DO);
        statementList();
        mustBe(T_OD);

        // Jump to the address of the loop condition.
        codegen_->emit(JUMP, conditionAddress);

        // Fill in the reserved address with the conditional jump instruction
        // to the operator following the loop.
        codegen_->emitAt(jumpNoAddress, JUMP_NO,
                         codegen_->getCurrentAddress());
    } else if (match(T_WRITE)) {
        mustBe(T_LPAREN);
        expression();
        mustBe(T_RPAREN);
        codegen_->emit(PRINT);
    } else {
        reportError("statement expected.");
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
void Parser::expression() {
    term();
    while (see(T_ADDOP)) {
        Arithmetic op = scanner_->GetArithmeticValue();
        next();
        term();

        if (op == A_PLUS) {
            codegen_->emit(ADD);
        } else {
            codegen_->emit(SUB);
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
void Parser::term() {
    factor();
    while (see(T_MULOP)) {
        Arithmetic op = scanner_->GetArithmeticValue();
        next();
        factor();

        if (op == A_MULTIPLY) {
            codegen_->emit(MULT);
        } else {
            codegen_->emit(DIV);
        }
    }
}

/*
 * Factor is described by the following rules:
 *  <factor> -> number | identifier | -<factor> | (<expression>) | READ
 */
void Parser::factor() {
    if (see(T_NUMBER)) {
        int value = scanner_->GetIntValue();
        next();
        codegen_->emit(PUSH, value);
    } else if (see(T_TRUE)) {
        next();
        codegen_->emit(PUSH, 1);
    } else if (see(T_FALSE)) {
        next();
        codegen_->emit(PUSH, 0);
    } else if (see(T_IDENTIFIER)) {
        int varAddress = findOrAddVariable(scanner_->GetStringValue());
        next();
        codegen_->emit(LOAD, varAddress);
    } else if (see(T_ADDOP) && scanner_->GetArithmeticValue() == A_MINUS) {
        next();
        factor();
        codegen_->emit(INVERT);
    } else if (match(T_LPAREN)) {
        logicalOrExpression();
        mustBe(T_RPAREN);
    } else if (match(T_NOT)) {
        factor();
        codegen_->emit(PUSH, 0);
        codegen_->emit(COMPARE, 0);
    } else if (match(T_READ)) {
        codegen_->emit(INPUT);
    } else {
        reportError("expression expected.");
    }
}

void Parser::logicalAndExpression() {
    relation();
    while (see(T_LAND)) {
        next();
        relation();
        codegen_->emit(MULT);
    }

    std::vector<int> jumpFalseAddresses;
    bool has_and = false;
    while (see(T_AND)) {
        has_and = true;
        next();

        codegen_->emit(PUSH, 0);
        codegen_->emit(COMPARE, 0);
        jumpFalseAddresses.push_back(codegen_->reserve());

        relation();
    }

    if (has_and) {
        codegen_->emit(PUSH, 0);
        codegen_->emit(COMPARE, 0);
        jumpFalseAddresses.push_back(codegen_->reserve());

        codegen_->emit(PUSH, 1);
        int jumpTrueAddress = codegen_->reserve();

        for (int jumpFalseAddress : jumpFalseAddresses) {
            codegen_->emitAt(jumpFalseAddress, JUMP_YES,
                             codegen_->getCurrentAddress());
        }

        codegen_->emit(PUSH, 0);
        codegen_->emitAt(jumpTrueAddress, JUMP, codegen_->getCurrentAddress());
    }
}

void Parser::logicalOrExpression() {
    logicalAndExpression();
    while (see(T_LOR)) {
        next();
        logicalAndExpression();
        codegen_->emit(ADD);
        codegen_->emit(PUSH, 0);
        codegen_->emit(COMPARE, 3);
    }

    std::vector<int> jumpTrueAddresses;
    bool has_or = false;
    while (see(T_OR)) {
        has_or = true;
        next();

        codegen_->emit(PUSH, 1);
        codegen_->emit(COMPARE, 0);
        jumpTrueAddresses.push_back(codegen_->reserve());

        relation();
    }

    if (has_or) {
        codegen_->emit(PUSH, 1);
        codegen_->emit(COMPARE, 0);
        jumpTrueAddresses.push_back(codegen_->reserve());

        codegen_->emit(PUSH, 0);
        int jumpFalseAddress = codegen_->reserve();

        for (int jumpFalseAddress : jumpTrueAddresses) {
            codegen_->emitAt(jumpFalseAddress, JUMP_YES,
                             codegen_->getCurrentAddress());
        }

        codegen_->emit(PUSH, 1);
        codegen_->emitAt(jumpFalseAddress, JUMP,
                         codegen_->getCurrentAddress());
    }
}

// The condition compares two expressions.
// Depending on the result of the comparison
// at the top of the stack it will be 0 or 1.
void Parser::relation() {
    expression();
    if (see(T_CMP)) {
        Cmp cmp = scanner_->GetCmpValue();
        next();
        expression();
        switch (cmp) {
        case C_EQ:
            codegen_->emit(COMPARE, 0);
            break;
        case C_NE:
            codegen_->emit(COMPARE, 1);
            break;
        case C_LT:
            codegen_->emit(COMPARE, 2);
            break;
        case C_GT:
            codegen_->emit(COMPARE, 3);
            break;
        case C_LE:
            codegen_->emit(COMPARE, 4);
            break;
        case C_GE:
            codegen_->emit(COMPARE, 5);
            break;
        };
    }
}

int Parser::findOrAddVariable(const std::string &var) {
    VarTable::iterator it = variables_.find(var);
    if (it == variables_.end()) {
        variables_[var] = lastVar_;
        return lastVar_++;
    } else {
        return it->second;
    }
}

void Parser::mustBe(Token t) {
    if (!match(t)) {
        error_ = true;

        std::ostringstream msg;
        msg << TokenToString(scanner_->GetCurrentToken()) << " found while "
            << TokenToString(t) << " expected.";
        reportError(msg.str());

        recover(t);
    }
}

void Parser::recover(Token t) {
    while (!see(t) && !see(T_EOF)) {
        next();
    }

    if (see(t)) {
        next();
    }
}
