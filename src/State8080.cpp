//
// Created by robin on 10/12/18.
//

#include "State8080.h"

State8080::State8080(unsigned int memory_size) {
    memory = new uint8_t[memory_size];
}

State8080::~State8080() {
    delete memory;
}

bool parity(int x, int length) {
    int p = 0;
    while(x > 0 && length-- > 0 ){
        p += 0x1 & x;
        x >>= 1;
    }
    return p % 2 == 0;
}

void State8080::setFlags(uint8_t result) {
    cc.z = result == 0;
    cc.s = (0x80 == (result & 0x80));
    cc.p = parity(result, 8);
}

uint8_t State8080::readMemory(uint16_t address) {
    return memory[address];
}

void State8080::logicFlagsA() {
    cc.cy = cc.ac = 0;
    cc.z = (a == 0);
    cc.s = (0x80 == (a & 0x80));
    cc.p = parity(a, 8);
}

void State8080::flagsZSP(uint8_t value) {
    cc.z = (value == 0);
    cc.s = (0x80 == (value & 0x80));
    cc.p = parity(value, 8);
}
