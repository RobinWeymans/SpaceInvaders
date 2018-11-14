//
// Created by Robin Weymans on 10/12/18.
//

#ifndef EMULATOR_MACHINE8080_H
#define EMULATOR_MACHINE8080_H

#include <string>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Config.hpp>
#include "State8080.h"

class Machine8080 {
public:
    Machine8080();
    State8080 state;
    sf::RenderWindow window;

    void loadFileAtMemory(const std::string& filename, const uint16_t offset);
    void prettyPrint() const;
    void startEmulation();
private:
    std::thread graphics_thread;
    std::thread emulation_thread;
    double screenRefreshRate;
    unsigned int screenScale;
    unsigned int windowHeight;
    unsigned int windowWidth;
    double frequency;
    bool isRunning;
    bool interupt_enable;
    bool interupt_waiting;
    int interuptNumber;

    void processInput();
    int emulateOpcode();
    void unimplementedInstruction();
    int disassembleOpcode(){return disassembleOpcode(state.pc);}
    int disassembleOpcode(const uint16_t memory_address) const;

    static unsigned char cycles8080[];
    static void runEmulation(Machine8080* const machine );
    void startEmulationThread();

    void generateInterupt();
};


#endif //EMULATOR_MACHINE8080_H
