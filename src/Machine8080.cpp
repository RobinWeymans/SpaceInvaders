//
// Created by Robin Weymans on 10/12/18.
//

#include "Machine8080.h"

#include <fstream>
#include <iostream>
#include <algorithm>

Machine8080::Machine8080(uint32_t memory_size) : state(memory_size) {
    screenScale = 1;
    windowHeight = 256 * screenScale;
    windowWidth = 224 * screenScale;
    frequency = 2000000.0; // 2MHz
    screenRefreshRate = 60.0; //60Hz
    window.create(sf::VideoMode(windowHeight, windowWidth), "Space invaders");
    isRunning = true;
}

void Machine8080::loadFileAtMemory(const std::string &filename, const uint16_t offset) {
    std::ifstream file(filename);
    if(!file.is_open()){
        std::cerr << "File " << filename << " not found." << std::endl;
        return;
    }

    char readByte;
    unsigned int pos = offset;
    while(file.get(readByte)){
        state.memory[pos++] = (uint8_t)readByte;
    }
}
void Machine8080::startEmulation() {
    startEmulationThread();

    auto lastRefresh = std::chrono::system_clock::now();
    while(window.isOpen()){

        //Process input
        processInput();

        auto thisRefresh = std::chrono::system_clock::now();
        if(((std::chrono::duration<double>)(thisRefresh - lastRefresh)).count() > 1.0 / screenRefreshRate){
            if(interupt_enable && !interupt_waiting){
                interuptNumber = interuptNumber == 1 ? 2 : 1;
                interupt_waiting = true;
            }


            auto pixels = new sf::Uint8[windowWidth*windowHeight*4];
            sf::Texture texture;
            texture.create(windowHeight, windowWidth);
            sf::Sprite sprite(texture);

            uint8_t* framebuffer = &state.memory[0x2400];

            for (int i = 0; i < windowHeight * windowWidth / 8; ++i) {
                for (int bit = 0; bit < 8; ++bit) {
                    int index = (8*i+bit)*4;
                    if( (framebuffer[i] >> bit) & 0x1  ){
                        pixels[index] = 0xFF;
                        pixels[index+1] = 0xFF;
                        pixels[index+2] = 0xFF;
                        pixels[index+3] = 0xFF;
                    }else{
                        pixels[index] = 0x00;
                        pixels[index+1] = 0x00;
                        pixels[index+2] = 0x00;
                        pixels[index+3] = 0xff;
                    }
                }
            }

            texture.update(pixels);
            window.draw(sprite);


            window.display();

            lastRefresh = thisRefresh;
        }
    }
    isRunning = false;
}
void Machine8080::processInput() {
    sf::Event event;
    while(window.pollEvent(event)){
        switch(event.type){
            case sf::Event::Closed: window.close(); break;
            case sf::Event::KeyPressed:{
                switch (event.key.code){
                    case EXIT: window.close(); break;
                    case COIN: std::cerr << "COIN" << std::endl; in_port |= 0x01; break;
                    case START: in_port |= 0x04; break;
                    case FIRE: std::cerr << "FIRE" << std::endl; in_port |= 0x10; break;
                    case LEFT: in_port |= 0x20; break;
                    case RIGHT: in_port |= 0x40; break;
                    default: break;
                }
            }
            case sf::Event::KeyReleased:{
                switch (event.key.code){
                    case COIN: in_port &= ~0x01; break;
                    case START: in_port &= ~0x04; break;
                    case FIRE: in_port &= ~0x10; break;
                    case LEFT: in_port &= ~0x20; break;
                    case RIGHT: in_port &= ~0x40; break;
                    default: break;
                }
            }
            default: break;
        }
    }
}
void Machine8080::startEmulationThread() {
    emulation_thread = std::move(std::thread(runEmulation, this));
}
void Machine8080::runEmulation(Machine8080 *const machine) {
    auto lastOpcode = std::chrono::system_clock::now();
    int cyclesToWait = 0;

    //run the program as long as the window is open
    while(machine->isRunning) {

        auto thisRun = std::chrono::system_clock::now();

        if (((std::chrono::duration<double>) (thisRun - lastOpcode)).count() > 1.0 / machine->frequency) {
            if (cyclesToWait == 0) {
                if(machine->interupt_waiting){
                    machine->generateInterupt();
                }
//                machine->disassembleOpcode();
                cyclesToWait = machine->emulateOpcode();
            }
            cyclesToWait--;
            lastOpcode = thisRun;
        }
    }
}
void Machine8080::prettyPrint() const {
    printf(" af   bc   de   hl   pc   sp   flags\n");
    printf("%02x   %02x%02x %02x%02x %02x%02x %04x %04x  ", state.a, state.b, state.c, state.d, state.e, state.h, state.l, state.pc, state.sp);
    if(state.cc.z) printf("z"); else printf(".");
    if(state.cc.s) printf("s"); else printf(".");
    if(state.cc.p) printf("p"); else printf(".");
    if(state.cc.ac) printf("a"); else printf(".");
    if(state.cc.cy) printf("c"); else printf(".");
    printf("\n\n");

    //print first 10 disassembled lines after pc
    uint16_t art_pc = state.pc;
    for(int i = 0; i<10; i++){
        art_pc += disassembleOpcode(art_pc);
    }
}
int Machine8080::emulateOpcode() {
    //prettyPrint();

    unsigned char opcode = state.memory[state.pc];
    switch(opcode){
        case 0x00: state.incrementPC(); break; //NOP
        case 0x01:
            state.c = state.getNextByte(1);
            state.b = state.getNextByte(2);
            state.incrementPC(3);
            break;
        case 0x02: unimplementedInstruction();
        case 0x03:
            state.incrementBC();
            state.incrementPC();
            break;
        case 0x04: unimplementedInstruction();
        case 0x05:
            state.b--;
            state.setFlags(state.b);
            state.incrementPC();
            break;
        case 0x06:
            state.b = state.getNextByte();
            state.incrementPC(2);
            break;
        case 0x07: unimplementedInstruction();
        case 0x08: unimplementedInstruction();
        case 0x09:{
            uint32_t res = state.getHL() + state.getBC();
            state.h = (res & 0xFF00) >> 8;
            state.l = res & 0xFF;
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
        case 0x0F:
        {
            uint8_t x = state.a;
            state.a = ((x & 1) << 7) | (x >> 1);
            state.cc.cy = (1 == (x&1));
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
        case 0x16: unimplementedInstruction();
        case 0x17: unimplementedInstruction();
        case 0x18: unimplementedInstruction();
        case 0x19:
        {
            uint32_t res = state.getHL() + state.getDE();
            state.h = (res & 0xff00) >> 8;
            state.l = res & 0xff;
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
        case 0x1F: unimplementedInstruction();

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
        case 0x29:
        {
            uint32_t hl = state.getHL();
            uint32_t res = hl + hl;
            state.h = (res & 0xff00) >> 8;
            state.l = res & 0xff;
            state.cc.cy = (res & 0xffff0000) != 0;
            state.incrementPC();
            break;
        }
        case 0x2A: unimplementedInstruction();
        case 0x2B: unimplementedInstruction();
        case 0x2C: unimplementedInstruction();
        case 0x2D: unimplementedInstruction();
        case 0x2E: state.l = state.getNextByte(); state.incrementPC(2); break;
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
        case 0x35: {
            uint8_t res = static_cast<uint8_t>(state.readMemoryAtHL() - 1);
            state.flagsZSP(res);
            state.writeMemoryAtHL(res);
            state.incrementPC();
            break;
        }
        case 0x36: // MVI M, address - write the next byte to memory at HL
            state.writeMemory(state.getHL(), state.getNextByte());
            state.pc++;
            break;
        case 0x37: state.cc.cy = 1; state.incrementPC(); break;
        case 0x38: unimplementedInstruction();
        case 0x39: unimplementedInstruction();
        case 0x3A:
            state.a = state.readMemory(state.getNextAddress());
            state.incrementPC(3);
            break;
        case 0x3B: unimplementedInstruction();
        case 0x3C: unimplementedInstruction();
        case 0x3D:
            state.a--;
            state.flagsZSP(state.a);
            state.incrementPC();
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
        case 0x46: unimplementedInstruction();
        case 0x47: unimplementedInstruction();
        case 0x48: unimplementedInstruction();
        case 0x49: unimplementedInstruction();
        case 0x4A: unimplementedInstruction();
        case 0x4B: unimplementedInstruction();
        case 0x4C: unimplementedInstruction();
        case 0x4D: unimplementedInstruction();
        case 0x4E: unimplementedInstruction();
        case 0x4F: state.c = state.a; state.incrementPC(); break;

        case 0x50: unimplementedInstruction();
        case 0x51: unimplementedInstruction();
        case 0x52: unimplementedInstruction();
        case 0x53: unimplementedInstruction();
        case 0x54: unimplementedInstruction();
        case 0x55: unimplementedInstruction();
        case 0x56: state.d = state.readMemoryAtHL(); state.incrementPC(); break;
        case 0x57: state.d = state.a; state.incrementPC(); break;
        case 0x58: unimplementedInstruction();
        case 0x59: unimplementedInstruction();
        case 0x5A: unimplementedInstruction();
        case 0x5B: unimplementedInstruction();
        case 0x5C: unimplementedInstruction();
        case 0x5D: unimplementedInstruction();
        case 0x5E: state.e = state.readMemoryAtHL(); state.incrementPC(); break;
        case 0x5F: state.e = state.a; state.incrementPC(); break;

        case 0x60: unimplementedInstruction();
        case 0x61: unimplementedInstruction();
        case 0x62: unimplementedInstruction();
        case 0x63: unimplementedInstruction();
        case 0x64: unimplementedInstruction();
        case 0x65: unimplementedInstruction();
        case 0x66: state.h = state.readMemoryAtHL(); state.incrementPC(); break;
        case 0x67: state.h = state.a; state.incrementPC(); break;
        case 0x68: unimplementedInstruction();
        case 0x69: unimplementedInstruction();
        case 0x6A: unimplementedInstruction();
        case 0x6B: unimplementedInstruction();
        case 0x6C: unimplementedInstruction();
        case 0x6D: unimplementedInstruction();
        case 0x6E: unimplementedInstruction();
        case 0x6F: state.l = state.a; state.incrementPC(); break;

        case 0x70: unimplementedInstruction();
        case 0x71: unimplementedInstruction();
        case 0x72: unimplementedInstruction();
        case 0x73: unimplementedInstruction();
        case 0x74: unimplementedInstruction();
        case 0x75: unimplementedInstruction();
        case 0x76: unimplementedInstruction();
        case 0x77: state.writeMemory(state.getHL(), state.a); state.incrementPC(); break;
        case 0x78: unimplementedInstruction();
        case 0x79: unimplementedInstruction();
        case 0x7A: state.a = state.d; state.incrementPC(); break;
        case 0x7B: state.a = state.e; state.incrementPC(); break;
        case 0x7C: state.a = state.h; state.incrementPC(); break;
        case 0x7D: state.a = state.l; state.incrementPC(); break;
        case 0x7E: state.a = state.readMemoryAtHL(); state.incrementPC(); break;
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
        case 0x9A: unimplementedInstruction();
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
        case 0xA7:
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
        case 0xAF:
            state.a ^= state.a;
            state.logicFlagsA();
            state.incrementPC();
            break;

        case 0xB0: unimplementedInstruction();
        case 0xB1: unimplementedInstruction();
        case 0xB2: unimplementedInstruction();
        case 0xB3: unimplementedInstruction();
        case 0xB4: unimplementedInstruction();
        case 0xB5: unimplementedInstruction();
        case 0xB6: state.a |= state.readMemoryAtHL(); state.logicFlagsA(); state.incrementPC(); break;
        case 0xB7: unimplementedInstruction();
        case 0xB8: unimplementedInstruction();
        case 0xB9: unimplementedInstruction();
        case 0xBA: unimplementedInstruction();
        case 0xBB: unimplementedInstruction();
        case 0xBC: unimplementedInstruction();
        case 0xBD: unimplementedInstruction();
        case 0xBE: unimplementedInstruction();
        case 0xBF: unimplementedInstruction();

        case 0xC0: unimplementedInstruction();
        case 0xC1:
            state.c = state.popByte();
            state.b = state.popByte();
            state.incrementPC();
            break;
        case 0xC2:
            if(state.cc.z == 0){
                state.pc = state.getNextAddress();
            } else {
                state.incrementPC(3);
            }
            break;
        case 0xC3: //JMP ADDRESS
            state.pc = state.getNextAddress();
            break;
        case 0xC4: unimplementedInstruction();
        case 0xC5:
            state.pushBC();
            state.incrementPC();
            break;
        case 0xC6:
        {
            uint16_t x = state.a + state.getNextByte();
            state.flagsZSP(x & 0xFF);
            state.cc.cy = (x > 0xFF);
            state.a = x & 0xFF;
            state.incrementPC(2);
            break;
        }
        case 0xC7: unimplementedInstruction();
        case 0xC8:
            if(state.cc.z) {
                state.pc = state.popAddress();
            }else{
                state.incrementPC();
            }
            break;
        case 0xC9: //RET
            state.pc = state.popAddress();
            break;
        case 0xCA: //JZ jump if zero
            if(state.cc.z)
                state.pc = state.getNextAddress();
            else
                state.incrementPC(3);
            break;
        case 0xCB: unimplementedInstruction();
        case 0xCC: unimplementedInstruction();
        case 0xCD: //unconditional CALL
        {
            uint16_t return_address = state.pc + (uint16_t)3; //
            state.memory[--state.sp] = (return_address >> 8) & 0xff; // Write first 8 bytes to memory
            state.memory[--state.sp] = return_address & 0xff; // Write the second half to memory
            state.pc = state.getNextAddress();
            break;
        }
        case 0xCE: unimplementedInstruction();
        case 0xCF: unimplementedInstruction();

        case 0xD0: unimplementedInstruction();
        case 0xD1:
            state.e = state.popByte();
            state.d = state.popByte();
            state.incrementPC();
            break;
        case 0xD2: //JNC
            if(!state.cc.cy)
                state.pc = state.getNextAddress();
            else
                state.incrementPC(3);
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
        case 0xD4: unimplementedInstruction();
        case 0xD5: // PUSH D
            state.pushDE();
            state.incrementPC();
            break;
        case 0xD6: unimplementedInstruction();
        case 0xD7: unimplementedInstruction();
        case 0xD8:
            if(state.cc.cy != 0){
                state.pc = state.popAddress();
            }else{
                state.incrementPC();
            }
            break;
        case 0xD9: unimplementedInstruction();
        case 0xDA:
            if(state.cc.cy){
                state.pc = state.getNextAddress();
            }else{
                state.incrementPC(3);
            }
            break;
        case 0xDB:{ // IN
//            std::cerr << "IN " << std::hex << (int) state.getNextByte() << std::endl;
            switch(state.getNextByte()){
                case 0: state.a = 1; break;
                case 1: state.a = in_port; break;
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
        case 0xDC: unimplementedInstruction();
        case 0xDD: unimplementedInstruction();
        case 0xDE: unimplementedInstruction();
        case 0xDF: unimplementedInstruction();

        case 0xE0: unimplementedInstruction();
        case 0xE1:
            state.l = state.popByte();
            state.h = state.popByte();
            state.incrementPC();
            break;
        case 0xE2: unimplementedInstruction();
        case 0xE3: unimplementedInstruction();
        case 0xE4: unimplementedInstruction();
        case 0xE5: // PUSH H
            state.pushHL();
            state.incrementPC();
            break;
        case 0xE6:
            state.a &= state.getNextByte();
            state.logicFlagsA();
            state.incrementPC(2);
            break;
        case 0xE7: unimplementedInstruction();
        case 0xE8: unimplementedInstruction();
        case 0xE9: unimplementedInstruction();
        case 0xEA: unimplementedInstruction();
        case 0xEB: //XCHG
            std::swap(state.h, state.d);
            std::swap(state.l, state.e);
            state.incrementPC();
            break;
        case 0xEC: unimplementedInstruction();
        case 0xED: unimplementedInstruction();
        case 0xEE: unimplementedInstruction();
        case 0xEF: unimplementedInstruction();

        case 0xF0: unimplementedInstruction();
        case 0xF1: //POP PSW
        {
            uint8_t psw = state.popByte();
            state.a = state.popByte();
            state.cc.z = 0x0 != (psw & 0x1);
            state.cc.s = 0x0 != (psw & (0x1 << 1));
            state.cc.p = 0x0 != (psw & (0x1 << 2));
            state.cc.cy = 0x0 != (psw & (0x1 << 3));
            state.cc.ac = 0x0 != (psw & (0x1 << 4));
            state.incrementPC();
            break;
        }
        case 0xF2: unimplementedInstruction();
        case 0xF3: unimplementedInstruction();
        case 0xF4: unimplementedInstruction();
        case 0xF5:
            state.pushPSW();
            state.incrementPC();
            break;
        case 0xF6: unimplementedInstruction();
        case 0xF7: unimplementedInstruction();
        case 0xF8: unimplementedInstruction();
        case 0xF9: unimplementedInstruction();
        case 0xFA: unimplementedInstruction();
        case 0xFB:
            interupt_enable = true;
            state.incrementPC();
            break;
        case 0xFC: unimplementedInstruction();
        case 0xFD: unimplementedInstruction();
        case 0xFE: {
            uint8_t x = state.a - state.getNextByte();
            state.setFlags(x);
            state.cc.cy = state.a < state.getNextByte();
            state.incrementPC(2);
            break;
        }
        case 0xFF: unimplementedInstruction();
        default: break;
    }
    //return 1;
    return cycles8080[opcode];
}
void Machine8080::unimplementedInstruction() {
    std::cerr << "Error unimplemented instruction:\n" << std::endl;
    disassembleOpcode();
    exit(1);
}

unsigned char Machine8080::cycles8080[] = {
        4,  10, 7,  5,  5,  5,  7,  4,      4,  10, 7,  5,  5,  5,  7,  4,  //0x00..0x0f
        4,  10, 7,  5,  5,  5,  7,  4,      4,  10, 7,  5,  5,  5,  7,  4,  //0x10..0x1f
        4,  10, 16, 5,  5,  5,  7,  4,      4,  10, 16, 5,  5,  5,  7,  4,  //0x20..0x2f
        4,  10, 13, 5,  10, 10, 10, 4,      4,  10, 13, 5,  5,  5,  7,  4,  //0x30..0x3f

        5,  5,  5,  5,  5,  5,  7,  5,      5,  5,  5,  5,  5,  5,  7,  5,  //0x40..0x4f
        5,  5,  5,  5,  5,  5,  7,  5,      5,  5,  5,  5,  5,  5,  7,  5,  //0x50..0x5f
        5,  5,  5,  5,  5,  5,  7,  5,      5,  5,  5,  5,  5,  5,  7,  5,  //0x60..0x6f
        7,  7,  7,  7,  7,  7,  7,  7,      5,  5,  5,  5,  5,  5,  7,  5,  //0x70..0x7f

        4,  4,  4,  4,  4,  4,  7,  4,      4,  4,  4,  4,  4,  4,  7,  4,  //0x80..0x8f
        4,  4,  4,  4,  4,  4,  7,  4,      4,  4,  4,  4,  4,  4,  7,  4,  //0x90..0x9f
        4,  4,  4,  4,  4,  4,  7,  4,      4,  4,  4,  4,  4,  4,  7,  4,  //0xa0..0xaf
        4,  4,  4,  4,  4,  4,  7,  4,      4,  4,  4,  4,  4,  4,  7,  4,  //0xb0..0xbf

        11, 10, 10, 10, 17, 11, 7,  11,     11, 10, 10, 10, 10, 17, 7,  11, //0xc0..0xcf
        11, 10, 10, 10, 17, 11, 7,  11,     11, 10, 10, 10, 10, 17, 7,  11, //0xd0..0xdf
        11, 10, 10, 18, 17, 11, 7,  11,     11,  5, 10, 5,  17, 17, 7,  11, //0xe0..0xef
        11, 10, 10, 4,  17, 11, 7,  11,     11,  5, 10, 4,  17, 17, 7,  11, //0xf0..0xff
};

void Machine8080::generateInterupt() {
    state.pushPC();
    state.pc = interuptNumber * 8;
    interupt_enable = interupt_waiting = false;
}

void Machine8080::dumpMemoryToFile(const std::string &filename) {
    std::ofstream file (filename, std::ofstream::binary);
    for(int i=0; i<0x10000; i++){
        file << state.memory++;
    }
    file.close();
}
