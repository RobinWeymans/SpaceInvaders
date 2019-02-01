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
