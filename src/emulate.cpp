#include "Machine8080.h"

uint8_t getLSB(uint16_t word){
    return (uint8_t)(word & (uint16_t)0x00ff);
}
uint8_t getMSB(uint16_t word){
    return (uint8_t)((word & (uint16_t)0xff00) >> 8);
}

int Machine8080::emulateOpcode() {
    //Debug part
    // disassembleOpcode();
    // printf("%02x   %02x%02x %02x%02x %02x%02x %04x %04x  ", state.a, state.b, state.c, state.d, state.e, state.h, state.l, state.pc, state.sp);
    // if(state.cc.z) printf("z"); else printf(".");
    // if(state.cc.s) printf("s"); else printf(".");
    // if(state.cc.p) printf("p"); else printf(".");
    // if(state.cc.ac) printf("a"); else printf(".");
    // if(state.cc.cy) printf("c"); else printf(".");
    // printf("\n");
    
    unsigned char opcode = state.memory[state.pc];
    switch(opcode){
        case 0x00: //NOP
            state.incrementPC();
            break;
        case 0x01: //LXI B,word- load the next 2 bytes in register pair BC
            state.c = state.getNextByte(1);
            state.b = state.getNextByte(2);
            state.incrementPC(3);
            break;
        case 0x02: //STAX B
        	state.writeMemory(state.getBC(), state.a);
        	state.incrementPC();
        	break;
        case 0x03: //INX B - increment register pair BC
            state.incrementBC();
            state.incrementPC();
            break;
        case 0x04: //INR B
            state.b++;
            state.flagsZSP(state.b);
            state.incrementPC();
            break;
        case 0x05: //DCR B - decrement single register B
            state.b--;
            state.setFlags(state.b);
            state.incrementPC();
            break;
        case 0x06: //MVI B,byte - load the next byte into single register B
            state.b = state.getNextByte();
            state.incrementPC(2);
            break;
        case 0x07: //RLC - rotate left over carry
        {
            uint8_t x = state.a;
            state.a = ((x & 0x80) >> 7) | (x << 1);
            state.cc.cy = x & 0x80;
            state.incrementPC();
            break;
        }
        case 0x08: //NOP
        	state.incrementPC();
        	break;
        case 0x09:{
            uint32_t res = state.getHL() + state.getBC();
            state.h = getMSB((uint16_t) res);
            state.l = getLSB((uint16_t) res);
            state.cc.cy = (res & 0xFFFF0000) > 0;
            state.incrementPC();
            break;
        }
        case 0x0A:
            state.a = state.readMemoryAtBC();
            state.incrementPC();
            break;
        case 0x0B: unimplementedInstruction();
        case 0x0C: unimplementedInstruction();
        case 0x0D:
            state.c--;
            state.setFlags(state.c);
            state.incrementPC();
            break;
        case 0x0E:
            state.c = state.getNextByte();
            state.incrementPC(2);
            break;
        case 0x0F: //RRC - rotate right into carry
        {
            uint8_t x = state.a;
            bool bit_zero = x & 0x1;
            state.a = (x >> 1) | (bit_zero << 7);
            state.cc.cy = bit_zero;
            state.incrementPC();
            break;
        }

        case 0x10: unimplementedInstruction();
        case 0x11:
            state.e = state.getNextByte();
            state.d = state.getNextByte(2);
            state.incrementPC(3);
            break;
        case 0x12: unimplementedInstruction();
        case 0x13:
            state.incrementDE();
            state.incrementPC();
            break;
        case 0x14: unimplementedInstruction();
        case 0x15: unimplementedInstruction();
        case 0x16: //MVI D,byte - load the next byte into single register D
            state.d = state.getNextByte();
            state.incrementPC(2);
            break;
        case 0x17: unimplementedInstruction();
        case 0x18: unimplementedInstruction();
        case 0x19:
        {
            uint32_t res = state.getHL() + state.getDE();
            state.h = getMSB((uint16_t) res);
            state.l = getLSB((uint16_t) res);
            state.cc.cy = (res & 0xffff0000) != 0;
            state.incrementPC();
            break;
        }
        case 0x1A:
            state.a = state.readMemoryAtDE();
            state.incrementPC();
            break;
        case 0x1B: unimplementedInstruction();
        case 0x1C: unimplementedInstruction();
        case 0x1D: unimplementedInstruction();
        case 0x1E: unimplementedInstruction();
        case 0x1F: //RAR - rotate right through carry
        {
            uint8_t x = state.a;
            //Carry is shifted in on the left
            state.a = (x >> 1) | (state.cc.cy << 7);
            //Bit 0 of x into carry
            state.cc.cy = 0x1 & x;
            state.incrementPC();
            break;
        }

        case 0x20: unimplementedInstruction();
        case 0x21:
            state.h = state.getNextByte(2);
            state.l = state.getNextByte();
            state.incrementPC(3);
            break;
        case 0x22: unimplementedInstruction();
        case 0x23:
            state.incrementHL();
            state.incrementPC();
            break;
        case 0x24: unimplementedInstruction();
        case 0x25: unimplementedInstruction();
        case 0x26:
            state.h = state.getNextByte();
            state.incrementPC(2);
            break;
        case 0x27: unimplementedInstruction();
        case 0x28: unimplementedInstruction();
        case 0x29: //DAD B
        {
            uint32_t hl = state.getHL();
            uint32_t res = hl + hl;
            state.h = getMSB((uint16_t)res);
            state.l = getLSB((uint16_t)res);
            state.cc.cy = (res & 0xffff0000) != 0;
            state.incrementPC();
            break;
        }
        case 0x2A: //LHLD
        {
            uint16_t offset = state.getNextAddress();
            state.l = state.readMemory(offset);
            state.h = state.readMemory(offset+1);
            state.incrementPC(3);
            break;
        }
        case 0x2B: //DCX H - decrement register pair HL
            state.decrementHL();
            state.incrementPC();
            break;
        case 0x2C: unimplementedInstruction();
        case 0x2D: unimplementedInstruction();
        case 0x2E:
            state.l = state.getNextByte();
            state.incrementPC(2);
            break;
        case 0x2F: unimplementedInstruction();

        case 0x30: unimplementedInstruction();
        case 0x31: //LXI SP ADDRESS
            state.sp = state.getNextAddress();
            state.incrementPC(3);
            break;
        case 0x32: // STA ADDRESS - Store a to memory
        {
            uint16_t addr = state.getNextAddress();
            state.writeAToMemory(addr);
            state.incrementPC(3);
            break;
        }
        case 0x33: unimplementedInstruction();
        case 0x34: unimplementedInstruction();
        case 0x35:
		{
            uint8_t res = state.readMemoryAtHL() - 1;
            state.flagsZSP(res);
            state.writeMemoryAtHL(res);
            state.incrementPC();
            break;
        }
        case 0x36: // MVI M, address - write the next byte to memory at HL
            state.writeMemoryAtHL(state.getNextByte());
            state.incrementPC(2);
            break;
        case 0x37:
            state.cc.cy = true;
            state.incrementPC();
            break;
        case 0x38: unimplementedInstruction();
        case 0x39: unimplementedInstruction();
        case 0x3A:
            state.a = state.readMemory(state.getNextAddress());
            state.incrementPC(3);
            break;
        case 0x3B: unimplementedInstruction();
        case 0x3C: //INR A - increment register a
            state.a++;
            state.flagsZSP(state.a);
            state.incrementPC();
            break;
        case 0x3D:
            state.a--;
            state.flagsZSP(state.a);
            state.incrementPC();
            break;
        case 0x3E:
            state.a = state.getNextByte();
            state.incrementPC(2);
            break;
        case 0x3F: unimplementedInstruction();

        case 0x40: unimplementedInstruction();
        case 0x41: unimplementedInstruction();
        case 0x42: unimplementedInstruction();
        case 0x43: unimplementedInstruction();
        case 0x44: unimplementedInstruction();
        case 0x45: unimplementedInstruction();
        case 0x46:
            state.b = state.readMemoryAtHL();
            state.incrementPC();
            break;
        case 0x47: //MOV B,A - load a into b
            state.b = state.a;
            state.incrementPC();
            break;
        case 0x48: unimplementedInstruction();
        case 0x49: unimplementedInstruction();
        case 0x4A: unimplementedInstruction();
        case 0x4B: unimplementedInstruction();
        case 0x4C: unimplementedInstruction();
        case 0x4D: unimplementedInstruction();
        case 0x4E: unimplementedInstruction();
        case 0x4F:
            state.c = state.a;
            state.incrementPC();
            break;

        case 0x50: unimplementedInstruction();
        case 0x51: unimplementedInstruction();
        case 0x52: unimplementedInstruction();
        case 0x53: unimplementedInstruction();
        case 0x54: unimplementedInstruction();
        case 0x55: unimplementedInstruction();
        case 0x56:
            state.d = state.readMemoryAtHL();
            state.incrementPC();
            break;
        case 0x57:
            state.d = state.a;
            state.incrementPC();
            break;
        case 0x58: unimplementedInstruction();
        case 0x59: unimplementedInstruction();
        case 0x5A: unimplementedInstruction();
        case 0x5B: unimplementedInstruction();
        case 0x5C: unimplementedInstruction();
        case 0x5D: unimplementedInstruction();
        case 0x5E:
            state.e = state.readMemoryAtHL();
            state.incrementPC();
            break;
        case 0x5F:
            state.e = state.a;
            state.incrementPC();
            break;

        case 0x60: unimplementedInstruction();
        case 0x61: unimplementedInstruction();
        case 0x62: unimplementedInstruction();
        case 0x63: unimplementedInstruction();
        case 0x64: unimplementedInstruction();
        case 0x65: unimplementedInstruction();
        case 0x66:
            state.h = state.readMemoryAtHL();
            state.incrementPC();
            break;
        case 0x67:
            state.h = state.a;
            state.incrementPC();
            break;
        case 0x68: unimplementedInstruction();
        case 0x69: unimplementedInstruction();
        case 0x6A: unimplementedInstruction();
        case 0x6B: unimplementedInstruction();
        case 0x6C: unimplementedInstruction();
        case 0x6D: unimplementedInstruction();
        case 0x6E: unimplementedInstruction();
        case 0x6F: //MOV L,A
            state.l = state.a;
            state.incrementPC();
            break;

        case 0x70: //MOV M,B
            state.writeMemoryAtHL(state.b);
            state.incrementPC();
            break;
        case 0x71: unimplementedInstruction();
        case 0x72: unimplementedInstruction();
        case 0x73: unimplementedInstruction();
        case 0x74: unimplementedInstruction();
        case 0x75: unimplementedInstruction();
        case 0x76: unimplementedInstruction();
        case 0x77: // MOV M,A
            state.writeMemoryAtHL(state.a);
            state.incrementPC();
            break;
        case 0x78: //MOV A,B - load register b into register a
            state.a = state.b;
            state.incrementPC();
            break;
        case 0x79: //MOV A,C - load register c into register a
            state.a = state.c;
            state.incrementPC();
            break;
        case 0x7A: //MOV A,D - load register d into register a
            state.a = state.d;
            state.incrementPC();
            break;
        case 0x7B: //MOV A,E - load register e into register a
            state.a = state.e;
            state.incrementPC();
            break;
        case 0x7C: //MOV A,H
            state.a = state.h;
            state.incrementPC();
            break;
        case 0x7D: //MOV A,L
            state.a = state.l;
            state.incrementPC();
            break;
        case 0x7E: //MOV A,M
            state.a = state.readMemoryAtHL();
            state.incrementPC();
            break;
        case 0x7F: unimplementedInstruction();

        case 0x80: unimplementedInstruction();
        case 0x81: unimplementedInstruction();
        case 0x82: unimplementedInstruction();
        case 0x83: unimplementedInstruction();
        case 0x84: unimplementedInstruction();
        case 0x85: unimplementedInstruction();
        case 0x86: unimplementedInstruction();
        case 0x87: unimplementedInstruction();
        case 0x88: unimplementedInstruction();
        case 0x89: unimplementedInstruction();
        case 0x8A: unimplementedInstruction();
        case 0x8B: unimplementedInstruction();
        case 0x8C: unimplementedInstruction();
        case 0x8D: unimplementedInstruction();
        case 0x8E: unimplementedInstruction();
        case 0x8F: unimplementedInstruction();

        case 0x90: unimplementedInstruction();
        case 0x91: unimplementedInstruction();
        case 0x92: unimplementedInstruction();
        case 0x93: unimplementedInstruction();
        case 0x94: unimplementedInstruction();
        case 0x95: unimplementedInstruction();
        case 0x96: unimplementedInstruction();
        case 0x97: unimplementedInstruction();
        case 0x98: unimplementedInstruction();
        case 0x99: unimplementedInstruction();
        case 0x9A: //SBB D
        {
            uint16_t res = state.a - state.d - state.cc.cy;
            state.arithFlagsA(res);
            state.a = (uint8_t)(res & 0xFF);
            state.incrementPC();
            break;
        }
        case 0x9B: unimplementedInstruction();
        case 0x9C: unimplementedInstruction();
        case 0x9D: unimplementedInstruction();
        case 0x9E: unimplementedInstruction();
        case 0x9F: unimplementedInstruction();

        case 0xA0: unimplementedInstruction();
        case 0xA1: unimplementedInstruction();
        case 0xA2: unimplementedInstruction();
        case 0xA3: unimplementedInstruction();
        case 0xA4: unimplementedInstruction();
        case 0xA5: unimplementedInstruction();
        case 0xA6: unimplementedInstruction();
        case 0xA7: //ANA A and a with a
            state.a &= state.a;
            state.logicFlagsA();
            state.incrementPC();
            break;
        case 0xA8: unimplementedInstruction();
        case 0xA9: unimplementedInstruction();
        case 0xAA: unimplementedInstruction();
        case 0xAB: unimplementedInstruction();
        case 0xAC: unimplementedInstruction();
        case 0xAD: unimplementedInstruction();
        case 0xAE: unimplementedInstruction();
        case 0xAF: //XRA A - exclusive OR a with a
            state.a ^= state.a;
            state.logicFlagsA();
            state.incrementPC();
            break;

        case 0xB0: //ORA B - logical OR a with b
            state.a |= state.b;
            state.logicFlagsA();
            state.incrementPC();
            break;
        case 0xB1: unimplementedInstruction();
        case 0xB2: unimplementedInstruction();
        case 0xB3: unimplementedInstruction();
        case 0xB4: unimplementedInstruction();
        case 0xB5: unimplementedInstruction();
        case 0xB6: //ORA M
            state.a |= state.readMemoryAtHL();
            state.logicFlagsA();
            state.incrementPC();
            break;
        case 0xB7: unimplementedInstruction();
        case 0xB8: unimplementedInstruction();
        case 0xB9: unimplementedInstruction();
        case 0xBA: unimplementedInstruction();
        case 0xBB: unimplementedInstruction();
        case 0xBC: unimplementedInstruction();
        case 0xBD: unimplementedInstruction();
        case 0xBE: unimplementedInstruction();
        case 0xBF: unimplementedInstruction();

        case 0xC0: //RZ - return if zero
            state.ret(state.cc.z);
            break;
        case 0xC1: //POP B
            state.c = state.popByte();
            state.b = state.popByte();
            state.incrementPC();
            break;
        case 0xC2: // JNZ - jump if not zero
            state.jump(!state.cc.z);
            break;
        case 0xC3: //JMP ADDRESS - unconditional jump
            state.pc = state.getNextAddress();
            break;
        case 0xC4: //CNZ - call if not zero
            state.call(!state.cc.z);
            break;
        case 0xC5:
            state.pushBC();
            state.incrementPC();
            break;
        case 0xC6: // ADI - add the next byte to a
        {
            uint16_t x = state.a + state.getNextByte();
            state.flagsZSP(x & 0xFF);
            state.cc.cy = (x > 0xFF);
            state.a = getLSB(x);
            state.incrementPC(2);
            break;
        }
        case 0xC7: unimplementedInstruction();
        case 0xC8: //RZ - return if zero
            state.ret(state.cc.z);
            break;
        case 0xC9: //RET - return to address on top of stack
            state.pc = state.popAddress();
            break;
        case 0xCA: //JZ - jump to address if zero
            state.jump(state.cc.z);
            break;
        case 0xCB: unimplementedInstruction();
        case 0xCC:
            state.call(state.cc.z);
            break;
        case 0xCD: //CALL - unconditional call
            state.call();
            break;
        case 0xCE: //ACI - add immediate to A with carry
        {
            uint16_t res = state.a + state.getNextByte() + state.cc.cy;
            state.flagsZSP(res & 0xFF);
            state.cc.cy = res > 0xFF;
            state.a = res & 0xFF;
            state.incrementPC(2);
            break;
        }
        case 0xCF: unimplementedInstruction();

        case 0xD0: //RNC - return if not carry
            state.ret(!state.cc.cy);
            break;
        case 0xD1:
            state.e = state.popByte();
            state.d = state.popByte();
            state.incrementPC();
            break;
        case 0xD2: //JNC
            state.jump(!state.cc.cy);
            break;
        case 0xD3: //OUT
        {
//            std::cerr << "OUT" << std::endl;
            switch(state.getNextByte()){
                case 2:
                    shift_offset = state.a & (uint8_t)0x7;
                    break;
                case 4:
                    shift0 = shift1;
                    shift1 = state.a;
                    break;
            }
        }
            state.incrementPC(2);
            break;
        case 0xD4:
            state.call(!state.cc.cy);
            break;
        case 0xD5: // PUSH D
            state.pushDE();
            state.incrementPC();
            break;
        case 0xD6: //SUI - subtract immediate register from A
        {
            uint8_t byte = state.getNextByte();
            uint8_t res = state.a - byte;
            state.flagsZSP(res);
            state.cc.cy = state.a < byte;
            state.a = res;
            state.incrementPC(2);
            break;
        }
        case 0xD7: unimplementedInstruction();
        case 0xD8: //RC - return if carry bit is set
            state.ret(state.cc.cy);
            break;
        case 0xD9: unimplementedInstruction();
        case 0xDA: //JC - jump if carry bit is set
            state.jump(state.cc.cy);
            break;
        case 0xDB:{ // IN
            switch(state.getNextByte()){
                case 0: state.a = 0xf; break;
                case 1: state.a = in_port; break;
                case 2: state.a = 0; break;
                case 3: {
                    uint16_t v = (shift1<<8) | shift0;
                    state.a = (v >> (8-shift_offset)) & 0xff;
                    break;
                }
                default: break;
            }
            state.incrementPC(2);
            break;
        }
        case 0xDC: //CC - call if carry flag set
            state.call(state.cc.cy);
            break;
        case 0xDD: unimplementedInstruction();
        case 0xDE: //SBI - Subtract immediate from A with borrow
        {
            uint8_t byte = state.getNextByte();
            uint16_t res = state.a - byte - state.cc.cy;
            state.flagsZSP(res & 0xFF);
            state.a = res & 0xFF;
            state.cc.cy = res > 0xFF;
            state.incrementPC(2);
            break;
        }
        case 0xDF: unimplementedInstruction();

        case 0xE0: unimplementedInstruction();
        case 0xE1:
            state.l = state.popByte();
            state.h = state.popByte();
            state.incrementPC();
            break;
        case 0xE2: //JPO - jump if parity flag is not set
            state.jump(!state.cc.p);
            break;
        case 0xE3: //XTHL - exchange top of stack with HL
        {
            std::swap(state.h, state.memory[state.sp]);
            std::swap(state.l, state.memory[state.sp+1]);
            state.incrementPC();
            break;
        }
        case 0xE4: //CPO - call if parity bit is not set
            state.call(!state.cc.p);
            break;
        case 0xE5: //PUSH H
            state.pushHL();
            state.incrementPC();
            break;
        case 0xE6: //ANI - AND immediate with register Ar
            state.a &= state.getNextByte();
            state.logicFlagsA();
            state.incrementPC(2);
            break;
        case 0xE7: unimplementedInstruction();
        case 0xE8: unimplementedInstruction();
        case 0xE9: // PCHL - load hl into pc
            state.pc = state.getHL();
            state.incrementPC();
            break;
        case 0xEA: //JPE
            state.jump(state.cc.p);
            break;
        case 0xEB: //XCHG
            std::swap(state.h, state.d);
            std::swap(state.l, state.e);
            state.incrementPC();
            break;
        case 0xEC://CPE -- call if parity flag set
            state.call(state.cc.p);
            break;
        case 0xED: unimplementedInstruction();
        case 0xEE: //XRI - ExclusiveOR immediate with A
        {
            uint8_t res = state.a ^ state.getNextByte();
            state.flagsZSP(res);
            state.a = res;
            state.cc.cy = false; // clear cy
            state.incrementPC(2);
            break;
        }
        case 0xEF: unimplementedInstruction();

        case 0xF0: unimplementedInstruction();
        case 0xF1: //POP PSW
        {
            uint8_t psw = state.popByte();
            state.a = state.popByte();
            state.cc.fromByte(psw);
            state.incrementPC();
            break;
        }
        case 0xF2: // JP - jump if (s)ign flag is not set
            state.jump(!state.cc.s);
            break;
        case 0xF3: unimplementedInstruction();
        case 0xF4:
            state.call(!state.cc.s);
            break;
        case 0xF5:
            state.pushPSW();
            state.incrementPC();
            break;
        case 0xF6: //ORI
        {
            uint8_t x = state.a | state.getNextByte();
            state.flagsZSP(x);
            state.cc.cy = false;
            state.a = x;
            state.incrementPC(2);
            break;
        }
        case 0xF7: unimplementedInstruction();
        case 0xF8: unimplementedInstruction();
        case 0xF9: unimplementedInstruction();
        case 0xFA: //JM - jump if the sign flag is set
            state.jump(state.cc.s);
            break;
        case 0xFB: //IE
            interupt_enable = true;
            state.incrementPC();
            break;
        case 0xFC: //CM - call if sign flag set
            state.call(state.cc.s);
            break;
        case 0xFD: unimplementedInstruction();
        case 0xFE: { //CPI - compare immediate with register A
            uint8_t x = state.a - state.getNextByte();
            state.flagsZSP(x);
            state.cc.cy = state.a < state.getNextByte();
            state.incrementPC(2);
            break;
        }
        case 0xFF: unimplementedInstruction();
        default: break;
    }
    return cycles8080[opcode];
}