//
// Created by robin on 10/12/18.
//

#ifndef EMULATOR_STATE8080_H
#define EMULATOR_STATE8080_H


#include <cstdint>
#include <iostream>

typedef struct {
    bool	cy:1;
    bool	pad:1;
    bool	p:1;
    bool	pad2:1;
    bool	ac:1;
    bool	pad3:1;
    bool	z:1;
    bool	s:1;
}ConditionCodes;

class State8080 {
public:
    ConditionCodes cc;
    uint8_t		*memory;

    uint16_t pc; //Program counter
    uint16_t sp; //Stack pointer

    //Registers
    uint8_t		a;
    uint8_t		b;
    uint8_t		c;
    uint8_t		d;
    uint8_t		e;
    uint8_t		h;
    uint8_t		l;

    explicit State8080(unsigned int memory_size);
    ~State8080();

    void setFlags(uint8_t result);
    uint8_t readMemory(uint16_t address);
    uint8_t readMemoryAtBC(){ return readMemory(getBC()); }
    uint8_t readMemoryAtDE(){ return readMemory(getDE()); }
    uint8_t readMemoryAtHL(){ return readMemory(getHL()); }

    void writeMemory(uint16_t address, uint8_t value){
        if(address < 0x2000){
            std::cerr << "Writng ROM not allowed!" << std::endl;
            exit(1);
        }
        if(address >= 0x4000){
            std::cerr << "Writing out of Space Invaders RAM not allowed!" << std::endl;
            exit(1);
        }
        memory[address] = value;
    }
    void writeMemoryAtHL(uint8_t value){writeMemory(getHL(), value);}
    void writeAToMemory(uint16_t address){ writeMemory(address, a); }

    uint8_t getNextByte(uint16_t i = 1){return readMemory(pc + i);}
    uint16_t getNextAddress(){return getNextByte(2) << 8 | getNextByte();}
    uint16_t getDE(){ return d << 8 | e; }
    uint16_t getHL(){ return h << 8 | l; }
    uint16_t getBC(){ return b << 8 | c; }
    uint8_t getPSW(){ return (cc.z | cc.s << 1 | cc.p << 2 | cc.cy << 3 | cc.ac << 4);}

    void push(uint8_t byte){writeMemory(--sp, byte);}
    void pushDE(){push(d); push(e);}
    void pushHL(){push(h); push(l);}
    void pushBC(){push(b); push(c);}
    void pushPSW(){ push(a); push(getPSW()); }
    void pushPC(){ push((pc & 0xFF00) >> 8); push(pc & 0xFF);}
    uint8_t popByte(){ return memory[sp++]; }
    uint16_t popAddress(){ return popByte() | popByte() << 8; }

    void incrementPC(uint16_t i=1){ pc += i; }
    void incrementDE(){ if(++e == 0) d++; }
    void incrementHL(){ if(++l == 0) h++; }

    void logicFlagsA();
    void flagsZSP(uint8_t);
};


#endif //EMULATOR_STATE8080_H
