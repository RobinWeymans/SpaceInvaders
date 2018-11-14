//
// Created by Robin Weymans on 10/12/18.
//

#include "Machine8080.h"

#include <fstream>
#include <iostream>
#include <algorithm>

Machine8080::Machine8080() : state(0x10000/* 16Kb of memory */) {
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
                    case sf::Keyboard::Escape: window.close(); break;
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
int Machine8080::disassembleOpcode(const uint16_t pc) const{
    unsigned char opcode = state.memory[pc];
    unsigned int opbytes = 1;
    printf("Ox%04x 0x%02x ", pc, opcode);
    switch(opcode){
        case 0x00: printf("NOP"); break;
        case 0x01: printf("LXI    B,#$%02x%02x", state.memory[pc+2], state.memory[pc+1]); opbytes=3; break;
        case 0x02: printf("STAX   B"); break;
        case 0x03: printf("INX    B"); break;
        case 0x04: printf("INR    B"); break;
        case 0x05: printf("DCR    B"); break;
        case 0x06: printf("MVI    B,#$%02x", state.memory[pc+1]); opbytes=2; break;
        case 0x07: printf("RLC"); break;
        case 0x08: printf("NOP"); break;
        case 0x09: printf("DAD    B"); break;
        case 0x0a: printf("LDAX   B"); break;
        case 0x0b: printf("DCX    B"); break;
        case 0x0c: printf("INR    C"); break;
        case 0x0d: printf("DCR    C"); break;
        case 0x0e: printf("MVI    C,#$%02x", state.memory[pc+1]); opbytes = 2; break;
        case 0x0f: printf("RRC"); break;

        case 0x10: printf("NOP"); break;
        case 0x11: printf("LXI    D,#$%02x%02x", state.memory[pc+2], state.memory[pc+1]); opbytes=3; break;
        case 0x12: printf("STAX   D"); break;
        case 0x13: printf("INX    D"); break;
        case 0x14: printf("INR    D"); break;
        case 0x15: printf("DCR    D"); break;
        case 0x16: printf("MVI    D,#$%02x", state.memory[pc+1]); opbytes=2; break;
        case 0x17: printf("RAL"); break;
        case 0x18: printf("NOP"); break;
        case 0x19: printf("DAD    D"); break;
        case 0x1a: printf("LDAX   D"); break;
        case 0x1b: printf("DCX    D"); break;
        case 0x1c: printf("INR    E"); break;
        case 0x1d: printf("DCR    E"); break;
        case 0x1e: printf("MVI    E,#$%02x", state.memory[pc+1]); opbytes = 2; break;
        case 0x1f: printf("RAR"); break;

        case 0x20: printf("NOP"); break;
        case 0x21: printf("LXI    H,#$%02x%02x", state.memory[pc+2], state.memory[pc+1]); opbytes=3; break;
        case 0x22: printf("SHLD   $%02x%02x", state.memory[pc+2], state.memory[pc+1]); opbytes=3; break;
        case 0x23: printf("INX    H"); break;
        case 0x24: printf("INR    H"); break;
        case 0x25: printf("DCR    H"); break;
        case 0x26: printf("MVI    H,#$%02x", state.memory[pc+1]); opbytes=2; break;
        case 0x27: printf("DAA"); break;
        case 0x28: printf("NOP"); break;
        case 0x29: printf("DAD    H"); break;
        case 0x2a: printf("LHLD   $%02x%02x", state.memory[pc+2], state.memory[pc+1]); opbytes=3; break;
        case 0x2b: printf("DCX    H"); break;
        case 0x2c: printf("INR    L"); break;
        case 0x2d: printf("DCR    L"); break;
        case 0x2e: printf("MVI    L,#$%02x", state.memory[pc+1]); opbytes = 2; break;
        case 0x2f: printf("CMA"); break;

        case 0x30: printf("NOP"); break;
        case 0x31: printf("LXI    SP,#$%02x%02x", state.memory[pc+2], state.memory[pc+1]); opbytes=3; break;
        case 0x32: printf("STA    $%02x%02x", state.memory[pc+2], state.memory[pc+1]); opbytes=3; break;
        case 0x33: printf("INX    SP"); break;
        case 0x34: printf("INR    M"); break;
        case 0x35: printf("DCR    M"); break;
        case 0x36: printf("MVI    M,#$%02x", state.memory[pc+1]); opbytes=2; break;
        case 0x37: printf("STC"); break;
        case 0x38: printf("NOP"); break;
        case 0x39: printf("DAD    SP"); break;
        case 0x3a: printf("LDA    $%02x%02x", state.memory[pc+2], state.memory[pc+1]); opbytes=3; break;
        case 0x3b: printf("DCX    SP"); break;
        case 0x3c: printf("INR    A"); break;
        case 0x3d: printf("DCR    A"); break;
        case 0x3e: printf("MVI    A,#$%02x", state.memory[pc+1]); opbytes = 2; break;
        case 0x3f: printf("CMC"); break;

        case 0x40: printf("MOV    B,B"); break;
        case 0x41: printf("MOV    B,C"); break;
        case 0x42: printf("MOV    B,D"); break;
        case 0x43: printf("MOV    B,E"); break;
        case 0x44: printf("MOV    B,H"); break;
        case 0x45: printf("MOV    B,L"); break;
        case 0x46: printf("MOV    B,M"); break;
        case 0x47: printf("MOV    B,A"); break;
        case 0x48: printf("MOV    C,B"); break;
        case 0x49: printf("MOV    C,C"); break;
        case 0x4a: printf("MOV    C,D"); break;
        case 0x4b: printf("MOV    C,E"); break;
        case 0x4c: printf("MOV    C,H"); break;
        case 0x4d: printf("MOV    C,L"); break;
        case 0x4e: printf("MOV    C,M"); break;
        case 0x4f: printf("MOV    C,A"); break;

        case 0x50: printf("MOV    D,B"); break;
        case 0x51: printf("MOV    D,C"); break;
        case 0x52: printf("MOV    D,D"); break;
        case 0x53: printf("MOV    D.E"); break;
        case 0x54: printf("MOV    D,H"); break;
        case 0x55: printf("MOV    D,L"); break;
        case 0x56: printf("MOV    D,M"); break;
        case 0x57: printf("MOV    D,A"); break;
        case 0x58: printf("MOV    E,B"); break;
        case 0x59: printf("MOV    E,C"); break;
        case 0x5a: printf("MOV    E,D"); break;
        case 0x5b: printf("MOV    E,E"); break;
        case 0x5c: printf("MOV    E,H"); break;
        case 0x5d: printf("MOV    E,L"); break;
        case 0x5e: printf("MOV    E,M"); break;
        case 0x5f: printf("MOV    E,A"); break;

        case 0x60: printf("MOV    H,B"); break;
        case 0x61: printf("MOV    H,C"); break;
        case 0x62: printf("MOV    H,D"); break;
        case 0x63: printf("MOV    H.E"); break;
        case 0x64: printf("MOV    H,H"); break;
        case 0x65: printf("MOV    H,L"); break;
        case 0x66: printf("MOV    H,M"); break;
        case 0x67: printf("MOV    H,A"); break;
        case 0x68: printf("MOV    L,B"); break;
        case 0x69: printf("MOV    L,C"); break;
        case 0x6a: printf("MOV    L,D"); break;
        case 0x6b: printf("MOV    L,E"); break;
        case 0x6c: printf("MOV    L,H"); break;
        case 0x6d: printf("MOV    L,L"); break;
        case 0x6e: printf("MOV    L,M"); break;
        case 0x6f: printf("MOV    L,A"); break;

        case 0x70: printf("MOV    M,B"); break;
        case 0x71: printf("MOV    M,C"); break;
        case 0x72: printf("MOV    M,D"); break;
        case 0x73: printf("MOV    M.E"); break;
        case 0x74: printf("MOV    M,H"); break;
        case 0x75: printf("MOV    M,L"); break;
        case 0x76: printf("HLT");        break;
        case 0x77: printf("MOV    M,A"); break;
        case 0x78: printf("MOV    A,B"); break;
        case 0x79: printf("MOV    A,C"); break;
        case 0x7a: printf("MOV    A,D"); break;
        case 0x7b: printf("MOV    A,E"); break;
        case 0x7c: printf("MOV    A,H"); break;
        case 0x7d: printf("MOV    A,L"); break;
        case 0x7e: printf("MOV    A,M"); break;
        case 0x7f: printf("MOV    A,A"); break;

        case 0x80: printf("ADD    B"); break;
        case 0x81: printf("ADD    C"); break;
        case 0x82: printf("ADD    D"); break;
        case 0x83: printf("ADD    E"); break;
        case 0x84: printf("ADD    H"); break;
        case 0x85: printf("ADD    L"); break;
        case 0x86: printf("ADD    M"); break;
        case 0x87: printf("ADD    A"); break;
        case 0x88: printf("ADC    B"); break;
        case 0x89: printf("ADC    C"); break;
        case 0x8a: printf("ADC    D"); break;
        case 0x8b: printf("ADC    E"); break;
        case 0x8c: printf("ADC    H"); break;
        case 0x8d: printf("ADC    L"); break;
        case 0x8e: printf("ADC    M"); break;
        case 0x8f: printf("ADC    A"); break;

        case 0x90: printf("SUB    B"); break;
        case 0x91: printf("SUB    C"); break;
        case 0x92: printf("SUB    D"); break;
        case 0x93: printf("SUB    E"); break;
        case 0x94: printf("SUB    H"); break;
        case 0x95: printf("SUB    L"); break;
        case 0x96: printf("SUB    M"); break;
        case 0x97: printf("SUB    A"); break;
        case 0x98: printf("SBB    B"); break;
        case 0x99: printf("SBB    C"); break;
        case 0x9a: printf("SBB    D"); break;
        case 0x9b: printf("SBB    E"); break;
        case 0x9c: printf("SBB    H"); break;
        case 0x9d: printf("SBB    L"); break;
        case 0x9e: printf("SBB    M"); break;
        case 0x9f: printf("SBB    A"); break;

        case 0xa0: printf("ANA    B"); break;
        case 0xa1: printf("ANA    C"); break;
        case 0xa2: printf("ANA    D"); break;
        case 0xa3: printf("ANA    E"); break;
        case 0xa4: printf("ANA    H"); break;
        case 0xa5: printf("ANA    L"); break;
        case 0xa6: printf("ANA    M"); break;
        case 0xa7: printf("ANA    A"); break;
        case 0xa8: printf("XRA    B"); break;
        case 0xa9: printf("XRA    C"); break;
        case 0xaa: printf("XRA    D"); break;
        case 0xab: printf("XRA    E"); break;
        case 0xac: printf("XRA    H"); break;
        case 0xad: printf("XRA    L"); break;
        case 0xae: printf("XRA    M"); break;
        case 0xaf: printf("XRA    A"); break;

        case 0xb0: printf("ORA    B"); break;
        case 0xb1: printf("ORA    C"); break;
        case 0xb2: printf("ORA    D"); break;
        case 0xb3: printf("ORA    E"); break;
        case 0xb4: printf("ORA    H"); break;
        case 0xb5: printf("ORA    L"); break;
        case 0xb6: printf("ORA    M"); break;
        case 0xb7: printf("ORA    A"); break;
        case 0xb8: printf("CMP    B"); break;
        case 0xb9: printf("CMP    C"); break;
        case 0xba: printf("CMP    D"); break;
        case 0xbb: printf("CMP    E"); break;
        case 0xbc: printf("CMP    H"); break;
        case 0xbd: printf("CMP    L"); break;
        case 0xbe: printf("CMP    M"); break;
        case 0xbf: printf("CMP    A"); break;

        case 0xc0: printf("RNZ"); break;
        case 0xc1: printf("POP    B"); break;
        case 0xc2: printf("JNZ    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xc3: printf("JMP    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xc4: printf("CNZ    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xc5: printf("PUSH   B"); break;
        case 0xc6: printf("ADI    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xc7: printf("RST    0"); break;
        case 0xc8: printf("RZ"); break;
        case 0xc9: printf("RET"); break;
        case 0xca: printf("JZ     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xcb: printf("JMP    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xcc: printf("CZ     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xcd: printf("CALL   $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xce: printf("ACI    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xcf: printf("RST    1"); break;

        case 0xd0: printf("RNC"); break;
        case 0xd1: printf("POP    D"); break;
        case 0xd2: printf("JNC    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xd3: printf("OUT    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xd4: printf("CNC    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xd5: printf("PUSH   D"); break;
        case 0xd6: printf("SUI    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xd7: printf("RST    2"); break;
        case 0xd8: printf("RC");  break;
        case 0xd9: printf("RET"); break;
        case 0xda: printf("JC     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xdb: printf("IN     #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xdc: printf("CC     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xdd: printf("CALL   $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xde: printf("SBI    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xdf: printf("RST    3"); break;

        case 0xe0: printf("RPO"); break;
        case 0xe1: printf("POP    H"); break;
        case 0xe2: printf("JPO    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xe3: printf("XTHL");break;
        case 0xe4: printf("CPO    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xe5: printf("PUSH   H"); break;
        case 0xe6: printf("ANI    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xe7: printf("RST    4"); break;
        case 0xe8: printf("RPE"); break;
        case 0xe9: printf("PCHL");break;
        case 0xea: printf("JPE    $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xeb: printf("XCHG"); break;
        case 0xec: printf("CPE     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xed: printf("CALL   $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xee: printf("XRI    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xef: printf("RST    5"); break;

        case 0xf0: printf("RP");  break;
        case 0xf1: printf("POP    PSW"); break;
        case 0xf2: printf("JP     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xf3: printf("DI");  break;
        case 0xf4: printf("CP     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xf5: printf("PUSH   PSW"); break;
        case 0xf6: printf("ORI    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xf7: printf("RST    6"); break;
        case 0xf8: printf("RM");  break;
        case 0xf9: printf("SPHL");break;
        case 0xfa: printf("JM     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xfb: printf("EI");  break;
        case 0xfc: printf("CM     $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xfd: printf("CALL   $%02x%02x",state.memory[pc+2],state.memory[pc+1]); opbytes = 3; break;
        case 0xfe: printf("CPI    #$%02x",state.memory[pc+1]); opbytes = 2; break;
        case 0xff: printf("RST    7"); break;

        default: break;
    }
    printf("\n");
    return opbytes;
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
        case 0x03: unimplementedInstruction();
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
        case 0x2E: unimplementedInstruction();
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
        case 0x37: state.cc.cy = 1; break;
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
        case 0x4F: unimplementedInstruction();

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
        case 0x57: unimplementedInstruction();
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
        case 0x5F: unimplementedInstruction();

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
        case 0x67: unimplementedInstruction();
        case 0x68: unimplementedInstruction();
        case 0x69: unimplementedInstruction();
        case 0x6A: unimplementedInstruction();
        case 0x6B: unimplementedInstruction();
        case 0x6C: unimplementedInstruction();
        case 0x6D: unimplementedInstruction();
        case 0x6E: unimplementedInstruction();
        case 0x6F:
            state.l = state.a;
            state.incrementPC();
            break;
        case 0x70: unimplementedInstruction();
        case 0x71: unimplementedInstruction();
        case 0x72: unimplementedInstruction();
        case 0x73: unimplementedInstruction();
        case 0x74: unimplementedInstruction();
        case 0x75: unimplementedInstruction();
        case 0x76: unimplementedInstruction();
        case 0x77:
            state.writeMemory(state.getHL(), state.a);
            state.incrementPC();
            break;
        case 0x78: unimplementedInstruction();
        case 0x79: unimplementedInstruction();
        case 0x7A:
            state.a = state.d;
            state.incrementPC();
            break;
        case 0x7B:
            state.a = state.e;
            state.incrementPC();
            break;
        case 0x7C:
            state.a = state.h;
            state.incrementPC();
            break;
        case 0x7D: unimplementedInstruction();
        case 0x7E:
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
        case 0xB6: unimplementedInstruction();
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
            if(state.cc.z) state.pc = state.popAddress();
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
            state.incrementPC(2);
            break;
        case 0xD4: unimplementedInstruction();
        case 0xD5: // PUSH D
            state.pushDE();
            state.incrementPC();
            break;
        case 0xD6: unimplementedInstruction();
        case 0xD7: unimplementedInstruction();
        case 0xD8: unimplementedInstruction();
        case 0xD9: unimplementedInstruction();
        case 0xDA:
            if(state.cc.cy){
                state.pc = state.getNextAddress();
            }else{
                state.incrementPC(3);
            }
            break;
        case 0xDB: // IN
            state.incrementPC(2);
            break;
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
    return 1;
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
