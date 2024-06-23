#ifndef CMILAN_CODEGEN_H
#define CMILAN_CODEGEN_H

#include <iostream>
#include <vector>

// Milan virtual machine instructions.
enum Instruction {
    NOP,
    // Stop vm, shut down program.
    STOP,
    // LOAD addr - load data word at adress addr onto the stack.
    LOAD,
    // STORE addr - store data word from top of the stack at address addr.
    STORE,
    // BLOAD addr - load data word at address addr onto the stack + value at the
    // top of the stack.
    BLOAD,
    // BSTORE addr - store data word from top of the stack at address addr +
    // value at the top of the stack.
    BSTORE,
    // PUSH n - push value n on the stack.
    PUSH,
    // Remove word from the stack.
    POP,
    // Copy word on the top of the stack.
    DUP,
    // Add two words from the stack and store the result on the stack.
    ADD,
    // Subtract two words from the stack and store the result on the stack.
    SUB,
    // Multiply two words from the stack and store the result on the stack.
    MULT,
    // Divide two words from the stack and store the result on the stack.
    DIV,
    // Change sign of the word on the stack.
    INVERT,
    // COMPARE cmp - compare two words from the stack with comparison operation
    // cmp and store the result on the stack.
    COMPARE,
    // JUMP addr - unconditional jump to address addr.
    JUMP,
    // JUMP_YES addr - jump to addr if 1 is on the top of the stack.
    JUMP_YES,
    // JUMP_NO addr - jump to addr if 0 is on the top of the stack.
    JUMP_NO,
    // Read integer from stdin and store on the stack.
    INPUT,
    // Print integer from the stack to stdout
    PRINT
};

struct Command {
    Command(Instruction instruction);
    Command(Instruction instruction, int arg);
    void print(int address, std::ostream &os);

    Instruction instruction;
    int argument = 0;
};

// Code generator.
// Used for:
// - Build a program for Milan virtual machine.
// - Keep track of the last instruction address.
// - Buffer the program and print to the output stream.
class CodeGen {
public:
    explicit CodeGen(std::ostream &output);

    // Append instruction without arguments to the program.
    void emit(Instruction instruction);

    // Append instruction with one arguments to the program.
    void emit(Instruction instruction, int arg);

    // Set instruction without arguments at the specified address.
    void emitAt(int address, Instruction instruction);

    // Set instruction with one argument at the specified address.
    void emitAt(int address, Instruction instruction, int arg);

    // Get address after the last instruction.
    int getCurrentAddress();

    // Generate an "empty" instruction (NOP) and return its address.
    int reserve();

    // Output instructions to the stream.
    void flush();

private:
    std::ostream &m_OutputStream;
    std::vector<Command> m_Commands;
};

#endif
