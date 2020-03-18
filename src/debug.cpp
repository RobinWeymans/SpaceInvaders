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
//    std::cout << "Processing command: " << cmd_to_process << std::endl;

//    std::istringstream iss(cmd_to_process);
//
//    std::string sub_cmd;
//    iss >> sub_cmd;
//    if(sub_cmd == "run" || sub_cmd == "r"){
//        int number_of_cycles;
//        iss >> number_of_cycles;
//        std::cout << "Running " << number_of_cycles << " cycles." << std::endl;
//
//        for(int i = 0; i<number_of_cycles; i++)
//            emulateOpcode();
//
//        prettyPrint();
//    }else{
//        std::cerr << "ERROR: " << sub_cmd << " is not a recognized command." << std::endl;
//    }
    
    disassembleOpcode();
    printf("  ", state.a, state.b, state.c, state.d, state.e, state.h, state.l, state.pc, state.sp);
    if(state.cc.z) printf("z"); else printf(".");
    if(state.cc.s) printf("s"); else printf(".");
    if(state.cc.p) printf("p"); else printf(".");
    if(state.cc.ac) printf("a"); else printf(".");
    if(state.cc.cy) printf("c"); else printf(".");
    printf("\n");
    emulateOpcode();
}

