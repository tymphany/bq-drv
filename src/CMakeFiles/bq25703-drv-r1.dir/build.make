# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_SOURCE_DIR = /home/ryder/rivian/sep_git/bq-drv

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ryder/rivian/sep_git/bq-drv

# Include any dependencies generated for this target.
include src/CMakeFiles/bq25703-drv-r1.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/bq25703-drv-r1.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/bq25703-drv-r1.dir/flags.make

src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o: src/CMakeFiles/bq25703-drv-r1.dir/flags.make
src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o: src/bq40z50_interface.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ryder/rivian/sep_git/bq-drv/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o -c /home/ryder/rivian/sep_git/bq-drv/src/bq40z50_interface.cc

src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.i"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ryder/rivian/sep_git/bq-drv/src/bq40z50_interface.cc > CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.i

src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.s"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ryder/rivian/sep_git/bq-drv/src/bq40z50_interface.cc -o CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.s

src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o.requires:

.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o.requires

src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o.provides: src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o.requires
	$(MAKE) -f src/CMakeFiles/bq25703-drv-r1.dir/build.make src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o.provides.build
.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o.provides

src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o.provides.build: src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o


src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o: src/CMakeFiles/bq25703-drv-r1.dir/flags.make
src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o: src/tps65987_interface.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ryder/rivian/sep_git/bq-drv/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o -c /home/ryder/rivian/sep_git/bq-drv/src/tps65987_interface.cc

src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.i"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ryder/rivian/sep_git/bq-drv/src/tps65987_interface.cc > CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.i

src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.s"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ryder/rivian/sep_git/bq-drv/src/tps65987_interface.cc -o CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.s

src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o.requires:

.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o.requires

src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o.provides: src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o.requires
	$(MAKE) -f src/CMakeFiles/bq25703-drv-r1.dir/build.make src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o.provides.build
.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o.provides

src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o.provides.build: src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o


src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o: src/CMakeFiles/bq25703-drv-r1.dir/flags.make
src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o: src/gpio_config.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ryder/rivian/sep_git/bq-drv/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o -c /home/ryder/rivian/sep_git/bq-drv/src/gpio_config.cc

src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.i"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ryder/rivian/sep_git/bq-drv/src/gpio_config.cc > CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.i

src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.s"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ryder/rivian/sep_git/bq-drv/src/gpio_config.cc -o CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.s

src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o.requires:

.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o.requires

src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o.provides: src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o.requires
	$(MAKE) -f src/CMakeFiles/bq25703-drv-r1.dir/build.make src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o.provides.build
.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o.provides

src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o.provides.build: src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o


src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o: src/CMakeFiles/bq25703-drv-r1.dir/flags.make
src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o: src/bq25703_drv.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ryder/rivian/sep_git/bq-drv/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o -c /home/ryder/rivian/sep_git/bq-drv/src/bq25703_drv.cc

src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.i"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ryder/rivian/sep_git/bq-drv/src/bq25703_drv.cc > CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.i

src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.s"
	cd /home/ryder/rivian/sep_git/bq-drv/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ryder/rivian/sep_git/bq-drv/src/bq25703_drv.cc -o CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.s

src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o.requires:

.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o.requires

src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o.provides: src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o.requires
	$(MAKE) -f src/CMakeFiles/bq25703-drv-r1.dir/build.make src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o.provides.build
.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o.provides

src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o.provides.build: src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o


# Object files for target bq25703-drv-r1
bq25703__drv__r1_OBJECTS = \
"CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o" \
"CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o" \
"CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o" \
"CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o"

# External object files for target bq25703-drv-r1
bq25703__drv__r1_EXTERNAL_OBJECTS =

src/bq25703-drv-r1: src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o
src/bq25703-drv-r1: src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o
src/bq25703-drv-r1: src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o
src/bq25703-drv-r1: src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o
src/bq25703-drv-r1: src/CMakeFiles/bq25703-drv-r1.dir/build.make
src/bq25703-drv-r1: src/CMakeFiles/bq25703-drv-r1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ryder/rivian/sep_git/bq-drv/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable bq25703-drv-r1"
	cd /home/ryder/rivian/sep_git/bq-drv/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bq25703-drv-r1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/bq25703-drv-r1.dir/build: src/bq25703-drv-r1

.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/build

src/CMakeFiles/bq25703-drv-r1.dir/requires: src/CMakeFiles/bq25703-drv-r1.dir/bq40z50_interface.cc.o.requires
src/CMakeFiles/bq25703-drv-r1.dir/requires: src/CMakeFiles/bq25703-drv-r1.dir/tps65987_interface.cc.o.requires
src/CMakeFiles/bq25703-drv-r1.dir/requires: src/CMakeFiles/bq25703-drv-r1.dir/gpio_config.cc.o.requires
src/CMakeFiles/bq25703-drv-r1.dir/requires: src/CMakeFiles/bq25703-drv-r1.dir/bq25703_drv.cc.o.requires

.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/requires

src/CMakeFiles/bq25703-drv-r1.dir/clean:
	cd /home/ryder/rivian/sep_git/bq-drv/src && $(CMAKE_COMMAND) -P CMakeFiles/bq25703-drv-r1.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/clean

src/CMakeFiles/bq25703-drv-r1.dir/depend:
	cd /home/ryder/rivian/sep_git/bq-drv && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ryder/rivian/sep_git/bq-drv /home/ryder/rivian/sep_git/bq-drv/src /home/ryder/rivian/sep_git/bq-drv /home/ryder/rivian/sep_git/bq-drv/src /home/ryder/rivian/sep_git/bq-drv/src/CMakeFiles/bq25703-drv-r1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/bq25703-drv-r1.dir/depend

