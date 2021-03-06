# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/robin/CLionProjects/SpaceInvaders

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/robin/CLionProjects/SpaceInvaders

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/robin/CLionProjects/SpaceInvaders/CMakeFiles /home/robin/CLionProjects/SpaceInvaders/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/robin/CLionProjects/SpaceInvaders/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named Emulator

# Build rule for target.
Emulator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 Emulator
.PHONY : Emulator

# fast build rule for target.
Emulator/fast:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/build
.PHONY : Emulator/fast

src/Machine8080.o: src/Machine8080.cpp.o

.PHONY : src/Machine8080.o

# target to build an object file
src/Machine8080.cpp.o:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/Machine8080.cpp.o
.PHONY : src/Machine8080.cpp.o

src/Machine8080.i: src/Machine8080.cpp.i

.PHONY : src/Machine8080.i

# target to preprocess a source file
src/Machine8080.cpp.i:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/Machine8080.cpp.i
.PHONY : src/Machine8080.cpp.i

src/Machine8080.s: src/Machine8080.cpp.s

.PHONY : src/Machine8080.s

# target to generate assembly for a file
src/Machine8080.cpp.s:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/Machine8080.cpp.s
.PHONY : src/Machine8080.cpp.s

src/State8080.o: src/State8080.cpp.o

.PHONY : src/State8080.o

# target to build an object file
src/State8080.cpp.o:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/State8080.cpp.o
.PHONY : src/State8080.cpp.o

src/State8080.i: src/State8080.cpp.i

.PHONY : src/State8080.i

# target to preprocess a source file
src/State8080.cpp.i:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/State8080.cpp.i
.PHONY : src/State8080.cpp.i

src/State8080.s: src/State8080.cpp.s

.PHONY : src/State8080.s

# target to generate assembly for a file
src/State8080.cpp.s:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/State8080.cpp.s
.PHONY : src/State8080.cpp.s

src/debug.o: src/debug.cpp.o

.PHONY : src/debug.o

# target to build an object file
src/debug.cpp.o:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/debug.cpp.o
.PHONY : src/debug.cpp.o

src/debug.i: src/debug.cpp.i

.PHONY : src/debug.i

# target to preprocess a source file
src/debug.cpp.i:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/debug.cpp.i
.PHONY : src/debug.cpp.i

src/debug.s: src/debug.cpp.s

.PHONY : src/debug.s

# target to generate assembly for a file
src/debug.cpp.s:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/debug.cpp.s
.PHONY : src/debug.cpp.s

src/disassemble.o: src/disassemble.cpp.o

.PHONY : src/disassemble.o

# target to build an object file
src/disassemble.cpp.o:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/disassemble.cpp.o
.PHONY : src/disassemble.cpp.o

src/disassemble.i: src/disassemble.cpp.i

.PHONY : src/disassemble.i

# target to preprocess a source file
src/disassemble.cpp.i:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/disassemble.cpp.i
.PHONY : src/disassemble.cpp.i

src/disassemble.s: src/disassemble.cpp.s

.PHONY : src/disassemble.s

# target to generate assembly for a file
src/disassemble.cpp.s:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/disassemble.cpp.s
.PHONY : src/disassemble.cpp.s

src/emulate.o: src/emulate.cpp.o

.PHONY : src/emulate.o

# target to build an object file
src/emulate.cpp.o:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/emulate.cpp.o
.PHONY : src/emulate.cpp.o

src/emulate.i: src/emulate.cpp.i

.PHONY : src/emulate.i

# target to preprocess a source file
src/emulate.cpp.i:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/emulate.cpp.i
.PHONY : src/emulate.cpp.i

src/emulate.s: src/emulate.cpp.s

.PHONY : src/emulate.s

# target to generate assembly for a file
src/emulate.cpp.s:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/emulate.cpp.s
.PHONY : src/emulate.cpp.s

src/main.o: src/main.cpp.o

.PHONY : src/main.o

# target to build an object file
src/main.cpp.o:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/main.cpp.o
.PHONY : src/main.cpp.o

src/main.i: src/main.cpp.i

.PHONY : src/main.i

# target to preprocess a source file
src/main.cpp.i:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/main.cpp.i
.PHONY : src/main.cpp.i

src/main.s: src/main.cpp.s

.PHONY : src/main.s

# target to generate assembly for a file
src/main.cpp.s:
	$(MAKE) -f CMakeFiles/Emulator.dir/build.make CMakeFiles/Emulator.dir/src/main.cpp.s
.PHONY : src/main.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... Emulator"
	@echo "... edit_cache"
	@echo "... src/Machine8080.o"
	@echo "... src/Machine8080.i"
	@echo "... src/Machine8080.s"
	@echo "... src/State8080.o"
	@echo "... src/State8080.i"
	@echo "... src/State8080.s"
	@echo "... src/debug.o"
	@echo "... src/debug.i"
	@echo "... src/debug.s"
	@echo "... src/disassemble.o"
	@echo "... src/disassemble.i"
	@echo "... src/disassemble.s"
	@echo "... src/emulate.o"
	@echo "... src/emulate.i"
	@echo "... src/emulate.s"
	@echo "... src/main.o"
	@echo "... src/main.i"
	@echo "... src/main.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

