#include <SFML/Graphics.hpp>
#include "Machine8080.h"

#include <iostream>
#include <bitset>

int main()
{

    Machine8080* machine = new Machine8080(0x10000);
    machine->loadFileAtMemory("../res/rom/invaders.h",0x0000);
    machine->loadFileAtMemory("../res/rom/invaders.g",0x0800);
    machine->loadFileAtMemory("../res/rom/invaders.f",0x1000);
    machine->loadFileAtMemory("../res/rom/invaders.e",0x1800);
//    machine->dumpMemoryToFile("../res/memdump");
    machine->startEmulation();
    return 0;
}