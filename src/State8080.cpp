//
// Created by robin on 10/12/18.
//

#include "State8080.h"

uint8_t ConditionCodes::toByte() {
	return cy | pad << 1 | p << 2 | pad2 << 3 | ac << 4 | pad3 << 5 | z << 6 | s << 7;
}

void ConditionCodes::fromByte(uint8_t byte) {
	cy	 = byte & (0x1 << 0);
	pad  = byte & (0x1 << 1);
	p 	 = byte & (0x1 << 2);
	pad2 = byte & (0x1 << 3);
	ac 	 = byte & (0x1 << 4);
	pad3 = byte & (0x1 << 5);
	z	 = byte & (0x1 << 6);
	s 	 = byte & (0x1 << 7);
}

State8080::State8080(unsigned int memory_size) : memory_size(memory_size){
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
    flagsZSP(a);
}

void State8080::arithFlagsA(uint16_t value) {
	cc.cy = value > 0xFF;
	cc.z = (value & 0xFF) == 0;
	cc.s = 0x80 == (value & 0x80);
	cc.p = parity(value & 0xFF, 8);
}

void State8080::flagsZSP(uint8_t value) {
    cc.z = (value == 0);
    cc.s = (0x80 == (value & 0x80));
    cc.p = parity(value, 8);
}

void State8080::call(bool condition){
	if(condition){
		uint16_t return_address = pc + 3;
		push((return_address >> 8) & 0xff);
		push(return_address & 0xff);
		pc = getNextAddress();
	}else{
		incrementPC(3);
	}
}

void State8080::jump(bool condition){
	if(condition){
		pc = getNextAddress();
	}else{
		incrementPC(3);
	}
}
void State8080::ret(bool condition){
	if(condition){
		pc = popAddress();
	}else{
		incrementPC();
	}
}