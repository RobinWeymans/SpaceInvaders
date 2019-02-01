//
// Created by robin on 2/1/19.
//

#include "Machine8080.h"

#include <sstream>

void Machine8080::enableDebugMode() {
    debug_mode = true;
}

void Machine8080::processDebugCommand(const std::string& cmd) {
    std::string cmd_to_process = std::string(cmd == "" ? default_command : cmd);
    std::cout << "Processing command: " << cmd_to_process << std::endl;

    std::istringstream iss(cmd_to_process);

    std::string sub_cmd;
    iss >> sub_cmd;
    if(sub_cmd == "run" || sub_cmd == "r"){
        int number_of_cycles;
        iss >> number_of_cycles;
        std::cout << "Running " << number_of_cycles << " cycles." << std::endl;

        for(int i = 0; i<number_of_cycles; i++)
            emulateOpcode();

        prettyPrint();
    }else{
        std::cerr << "ERROR: " << sub_cmd << " is not a recognized command." << std::endl;
    }

}

