#include <SFML/Graphics.hpp>
#include "Machine8080.h"

#include <iostream>
#include <bitset>

#define SPACEINVADERS

int main()
{
	Machine8080* machine = new Machine8080(0x10000);

#ifdef SPACEINVADERS
    machine->loadFileAtMemory("res/rom/invaders.h", 0x0000);
    machine->loadFileAtMemory("res/rom/invaders.g", 0x0800);
    machine->loadFileAtMemory("res/rom/invaders.f", 0x1000);
    machine->loadFileAtMemory("res/rom/invaders.e", 0x1800);
    machine->startEmulation();
#else
    machine->loadFileAtMemory("res/rom/cpudiag.bin", 0x100);
    machine->state.pc = 0x100;
	machine->state.memory[368] = 0x7;
	machine->state.memory[0x59c] = 0xc3; //JMP
	machine->state.memory[0x59d] = 0xc2;
	machine->state.memory[0x59e] = 0x05;
//    machine->enableDebugMode();
    machine->startEmulation();
#endif
    return 0;
}