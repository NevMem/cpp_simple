# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_COMMAND = /Users/yaigor/Library/Android/sdk/cmake/3.10.2.4988404/bin/cmake

# The command to remove a file.
RM = /Users/yaigor/Library/Android/sdk/cmake/3.10.2.4988404/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/yaigor/proj/cpp/adv

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/yaigor/proj/cpp/adv

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/Users/yaigor/Library/Android/sdk/cmake/3.10.2.4988404/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/Users/yaigor/Library/Android/sdk/cmake/3.10.2.4988404/bin/ccmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/yaigor/proj/cpp/adv/CMakeFiles /Users/yaigor/proj/cpp/adv/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/yaigor/proj/cpp/adv/CMakeFiles 0
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
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named result.o

# Build rule for target.
result.o: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 result.o
.PHONY : result.o

# fast build rule for target.
result.o/fast:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/build
.PHONY : result.o/fast

src/comparator.o: src/comparator.cpp.o

.PHONY : src/comparator.o

# target to build an object file
src/comparator.cpp.o:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/comparator.cpp.o
.PHONY : src/comparator.cpp.o

src/comparator.i: src/comparator.cpp.i

.PHONY : src/comparator.i

# target to preprocess a source file
src/comparator.cpp.i:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/comparator.cpp.i
.PHONY : src/comparator.cpp.i

src/comparator.s: src/comparator.cpp.s

.PHONY : src/comparator.s

# target to generate assembly for a file
src/comparator.cpp.s:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/comparator.cpp.s
.PHONY : src/comparator.cpp.s

src/impl/threading/dispatcher/dispatchers.o: src/impl/threading/dispatcher/dispatchers.cpp.o

.PHONY : src/impl/threading/dispatcher/dispatchers.o

# target to build an object file
src/impl/threading/dispatcher/dispatchers.cpp.o:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/impl/threading/dispatcher/dispatchers.cpp.o
.PHONY : src/impl/threading/dispatcher/dispatchers.cpp.o

src/impl/threading/dispatcher/dispatchers.i: src/impl/threading/dispatcher/dispatchers.cpp.i

.PHONY : src/impl/threading/dispatcher/dispatchers.i

# target to preprocess a source file
src/impl/threading/dispatcher/dispatchers.cpp.i:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/impl/threading/dispatcher/dispatchers.cpp.i
.PHONY : src/impl/threading/dispatcher/dispatchers.cpp.i

src/impl/threading/dispatcher/dispatchers.s: src/impl/threading/dispatcher/dispatchers.cpp.s

.PHONY : src/impl/threading/dispatcher/dispatchers.s

# target to generate assembly for a file
src/impl/threading/dispatcher/dispatchers.cpp.s:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/impl/threading/dispatcher/dispatchers.cpp.s
.PHONY : src/impl/threading/dispatcher/dispatchers.cpp.s

src/main.o: src/main.cpp.o

.PHONY : src/main.o

# target to build an object file
src/main.cpp.o:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/main.cpp.o
.PHONY : src/main.cpp.o

src/main.i: src/main.cpp.i

.PHONY : src/main.i

# target to preprocess a source file
src/main.cpp.i:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/main.cpp.i
.PHONY : src/main.cpp.i

src/main.s: src/main.cpp.s

.PHONY : src/main.s

# target to generate assembly for a file
src/main.cpp.s:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/main.cpp.s
.PHONY : src/main.cpp.s

src/sinlge_thread.o: src/sinlge_thread.cpp.o

.PHONY : src/sinlge_thread.o

# target to build an object file
src/sinlge_thread.cpp.o:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/sinlge_thread.cpp.o
.PHONY : src/sinlge_thread.cpp.o

src/sinlge_thread.i: src/sinlge_thread.cpp.i

.PHONY : src/sinlge_thread.i

# target to preprocess a source file
src/sinlge_thread.cpp.i:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/sinlge_thread.cpp.i
.PHONY : src/sinlge_thread.cpp.i

src/sinlge_thread.s: src/sinlge_thread.cpp.s

.PHONY : src/sinlge_thread.s

# target to generate assembly for a file
src/sinlge_thread.cpp.s:
	$(MAKE) -f CMakeFiles/result.o.dir/build.make CMakeFiles/result.o.dir/src/sinlge_thread.cpp.s
.PHONY : src/sinlge_thread.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... result.o"
	@echo "... src/comparator.o"
	@echo "... src/comparator.i"
	@echo "... src/comparator.s"
	@echo "... src/impl/threading/dispatcher/dispatchers.o"
	@echo "... src/impl/threading/dispatcher/dispatchers.i"
	@echo "... src/impl/threading/dispatcher/dispatchers.s"
	@echo "... src/main.o"
	@echo "... src/main.i"
	@echo "... src/main.s"
	@echo "... src/sinlge_thread.o"
	@echo "... src/sinlge_thread.i"
	@echo "... src/sinlge_thread.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

