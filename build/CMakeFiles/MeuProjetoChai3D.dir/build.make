# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/igor/MeuProjetoChai3d

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/igor/MeuProjetoChai3d/build

# Include any dependencies generated for this target.
include CMakeFiles/MeuProjetoChai3D.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/MeuProjetoChai3D.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/MeuProjetoChai3D.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MeuProjetoChai3D.dir/flags.make

CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o: CMakeFiles/MeuProjetoChai3D.dir/flags.make
CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o: /home/igor/MeuProjetoChai3d/src/main.cpp
CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o: CMakeFiles/MeuProjetoChai3D.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/igor/MeuProjetoChai3d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o -MF CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o.d -o CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o -c /home/igor/MeuProjetoChai3d/src/main.cpp

CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/igor/MeuProjetoChai3d/src/main.cpp > CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.i

CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/igor/MeuProjetoChai3d/src/main.cpp -o CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.s

CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o: CMakeFiles/MeuProjetoChai3D.dir/flags.make
CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o: /home/igor/MeuProjetoChai3d/src/config_parser.cpp
CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o: CMakeFiles/MeuProjetoChai3D.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/igor/MeuProjetoChai3d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o -MF CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o.d -o CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o -c /home/igor/MeuProjetoChai3d/src/config_parser.cpp

CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/igor/MeuProjetoChai3d/src/config_parser.cpp > CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.i

CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/igor/MeuProjetoChai3d/src/config_parser.cpp -o CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.s

CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o: CMakeFiles/MeuProjetoChai3D.dir/flags.make
CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o: /home/igor/MeuProjetoChai3d/src/haptic_simulator.cpp
CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o: CMakeFiles/MeuProjetoChai3D.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/igor/MeuProjetoChai3d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o -MF CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o.d -o CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o -c /home/igor/MeuProjetoChai3d/src/haptic_simulator.cpp

CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/igor/MeuProjetoChai3d/src/haptic_simulator.cpp > CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.i

CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/igor/MeuProjetoChai3d/src/haptic_simulator.cpp -o CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.s

# Object files for target MeuProjetoChai3D
MeuProjetoChai3D_OBJECTS = \
"CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o" \
"CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o" \
"CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o"

# External object files for target MeuProjetoChai3D
MeuProjetoChai3D_EXTERNAL_OBJECTS =

MeuProjetoChai3D: CMakeFiles/MeuProjetoChai3D.dir/src/main.cpp.o
MeuProjetoChai3D: CMakeFiles/MeuProjetoChai3D.dir/src/config_parser.cpp.o
MeuProjetoChai3D: CMakeFiles/MeuProjetoChai3D.dir/src/haptic_simulator.cpp.o
MeuProjetoChai3D: CMakeFiles/MeuProjetoChai3D.dir/build.make
MeuProjetoChai3D: /usr/lib/x86_64-linux-gnu/libGLEW.so
MeuProjetoChai3D: /usr/lib/x86_64-linux-gnu/libjsoncpp.so
MeuProjetoChai3D: libimgui.a
MeuProjetoChai3D: libimgui_impl_glfw.a
MeuProjetoChai3D: libimgui_impl_opengl3.a
MeuProjetoChai3D: /usr/lib/x86_64-linux-gnu/libGLX.so
MeuProjetoChai3D: /usr/lib/x86_64-linux-gnu/libOpenGL.so
MeuProjetoChai3D: libimgui.a
MeuProjetoChai3D: CMakeFiles/MeuProjetoChai3D.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/igor/MeuProjetoChai3d/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable MeuProjetoChai3D"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MeuProjetoChai3D.dir/link.txt --verbose=$(VERBOSE)
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold "Copiando fontes para o diretório do executável"
	/usr/bin/cmake -E copy_directory /home/igor/MeuProjetoChai3d/fonts /home/igor/MeuProjetoChai3d/build/fonts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold "Copiando assets para o diretório do executável"
	/usr/bin/cmake -E copy_directory /home/igor/MeuProjetoChai3d/assets /home/igor/MeuProjetoChai3d/build/assets

# Rule to build all files generated by this target.
CMakeFiles/MeuProjetoChai3D.dir/build: MeuProjetoChai3D
.PHONY : CMakeFiles/MeuProjetoChai3D.dir/build

CMakeFiles/MeuProjetoChai3D.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MeuProjetoChai3D.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MeuProjetoChai3D.dir/clean

CMakeFiles/MeuProjetoChai3D.dir/depend:
	cd /home/igor/MeuProjetoChai3d/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/igor/MeuProjetoChai3d /home/igor/MeuProjetoChai3d /home/igor/MeuProjetoChai3d/build /home/igor/MeuProjetoChai3d/build /home/igor/MeuProjetoChai3d/build/CMakeFiles/MeuProjetoChai3D.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/MeuProjetoChai3D.dir/depend

