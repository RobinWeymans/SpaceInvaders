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

#define EXIT sf::Keyboard::Escape
#define COIN sf::Keyboard::C
#define LEFT sf::Keyboard::Left
#define RIGHT sf::Keyboard::Right
#define FIRE sf::Keyboard::Space
#define START sf::Keyboard::Enter

class Machine8080 {
public:
    Machine8080(uint32_t memory_size);

    State8080 state;
    uint8_t in_port;
    uint8_t shift0, shift1, shift_offset;

    sf::RenderWindow window;

    void loadFileAtMemory(const std::string& filename, const uint16_t offset);
    void dumpMemoryToFile(const std::string& filename);
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
