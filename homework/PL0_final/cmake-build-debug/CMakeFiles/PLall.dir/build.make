# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.19

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "D:\Clion\CLion 2021.1.2\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "D:\Clion\CLion 2021.1.2\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = D:\Clion\PLall

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\Clion\PLall\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/PLall.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/PLall.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/PLall.dir/flags.make

CMakeFiles/PLall.dir/pl0.c.obj: CMakeFiles/PLall.dir/flags.make
CMakeFiles/PLall.dir/pl0.c.obj: CMakeFiles/PLall.dir/includes_C.rsp
CMakeFiles/PLall.dir/pl0.c.obj: ../pl0.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\Clion\PLall\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/PLall.dir/pl0.c.obj"
	X:\Dev_C++\Dev-Cpp\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\PLall.dir\pl0.c.obj -c D:\Clion\PLall\pl0.c

CMakeFiles/PLall.dir/pl0.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/PLall.dir/pl0.c.i"
	X:\Dev_C++\Dev-Cpp\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\Clion\PLall\pl0.c > CMakeFiles\PLall.dir\pl0.c.i

CMakeFiles/PLall.dir/pl0.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/PLall.dir/pl0.c.s"
	X:\Dev_C++\Dev-Cpp\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S D:\Clion\PLall\pl0.c -o CMakeFiles\PLall.dir\pl0.c.s

# Object files for target PLall
PLall_OBJECTS = \
"CMakeFiles/PLall.dir/pl0.c.obj"

# External object files for target PLall
PLall_EXTERNAL_OBJECTS =

PLall.exe: CMakeFiles/PLall.dir/pl0.c.obj
PLall.exe: CMakeFiles/PLall.dir/build.make
PLall.exe: CMakeFiles/PLall.dir/linklibs.rsp
PLall.exe: CMakeFiles/PLall.dir/objects1.rsp
PLall.exe: CMakeFiles/PLall.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\Clion\PLall\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable PLall.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\PLall.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/PLall.dir/build: PLall.exe

.PHONY : CMakeFiles/PLall.dir/build

CMakeFiles/PLall.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\PLall.dir\cmake_clean.cmake
.PHONY : CMakeFiles/PLall.dir/clean

CMakeFiles/PLall.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" D:\Clion\PLall D:\Clion\PLall D:\Clion\PLall\cmake-build-debug D:\Clion\PLall\cmake-build-debug D:\Clion\PLall\cmake-build-debug\CMakeFiles\PLall.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/PLall.dir/depend

