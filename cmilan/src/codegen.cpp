#include "codegen.h"

Command::Command(Instruction instruction) : instruction(instruction) {}

Command::Command(Instruction instruction, int arg)
    : instruction(instruction), argument(arg) {}

void Command::print(int address, std::ostream &os) {
    os << address << ":\t";
    switch (instruction) {
    case NOP:
        os << "NOP";
        break;

    case STOP:
        os << "STOP";
        break;

    case LOAD:
        os << "LOAD\t" << argument;
        break;

    case STORE:
        os << "STORE\t" << argument;
        break;

    case BLOAD:
        os << "BLOAD\t" << argument;
        break;

    case BSTORE:
        os << "BSTORE\t" << argument;
        break;

    case PUSH:
        os << "PUSH\t" << argument;
        break;

    case POP:
        os << "POP";
        break;

    case DUP:
        os << "DUP";
        break;

    case ADD:
        os << "ADD";
        break;

    case SUB:
        os << "SUB";
        break;

    case MULT:
        os << "MULT";
        break;

    case DIV:
        os << "DIV";
        break;

    case INVERT:
        os << "INVERT";
        break;

    case COMPARE:
        os << "COMPARE\t" << argument;
        break;

    case JUMP:
        os << "JUMP\t" << argument;
        break;

    case JUMP_YES:
        os << "JUMP_YES\t" << argument;
        break;

    case JUMP_NO:
        os << "JUMP_NO\t" << argument;
        break;

    case INPUT:
        os << "INPUT";
        break;

    case PRINT:
        os << "PRINT";
        break;
    }

    os << std::endl;
}

CodeGen::CodeGen(std::ostream &output) : m_OutputStream(output) {}

void CodeGen::emit(Instruction instruction) {
    m_Commands.push_back(Command(instruction));
}

void CodeGen::emit(Instruction instruction, int arg) {
    m_Commands.push_back(Command(instruction, arg));
}

void CodeGen::emitAt(int address, Instruction instruction) {
    m_Commands[address] = Command(instruction);
}

void CodeGen::emitAt(int address, Instruction instruction, int arg) {
    m_Commands[address] = Command(instruction, arg);
}

int CodeGen::getCurrentAddress() {
    return m_Commands.size();
}

int CodeGen::reserve() {
    emit(NOP);
    return m_Commands.size() - 1;
}

void CodeGen::flush() {
    int count = m_Commands.size();
    for (int address = 0; address < count; ++address) {
        m_Commands[address].print(address, m_OutputStream);
    }
    m_OutputStream.flush();
}
