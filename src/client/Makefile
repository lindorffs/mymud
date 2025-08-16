# Compiler
CXX = x86_64-w64-mingw32-g++
# For C files, we'll use CXX as well, assuming g++ can handle .c files
# or that CFLAGS compatible with C++ are used. If you need a specific C compiler:
# CC_C = x86_64-w64-mingw32-gcc

# Compile flags
CXXFLAGS = -Wa,-mbig-obj -Wextra -std=c++17 -g # Added -g for debug symbols
# CFLAGS_C = -Wall -std=c11 -g # Example if using a separate C compiler or flags

# --- Directories ---
OUTPUT_DIR = bin
OBJ_DIR = obj
BUILD_DIR = build

# --- Executable ---
TARGET_EXE_NAME = client.exe
TARGET = $(OUTPUT_DIR)/$(TARGET_EXE_NAME)

# --- Source Files ---
# Core engine C++ files
CORE_CPP_FILES = $(wildcard Core/*.cpp)
GAMEOBJECTS_CPP_FILES = $(wildcard GameObjects/*.cpp)
WORLD_CPP_FILES = $(wildcard World/*.cpp)
UTILS_CPP_FILES = $(wildcard Utils/*.cpp)
UI_CPP_FILES = $(wildcard UI/*.cpp)
IMGUI_CPP_FILES = $(wildcard ../../vendor/imgui/*.cpp) ../../vendor/imgui/backends/imgui_impl_sdl3.cpp ../../vendor/imgui/backends/imgui_impl_sdlrenderer3.cpp
MAIN_CPP_FILE = main.cpp

# Combine all C++ sources
CPP_SOURCES = \
    $(MAIN_CPP_FILE) \
    $(CORE_CPP_FILES) \
    $(GAMEOBJECTS_CPP_FILES) \
    $(WORLD_CPP_FILES) \
    $(UTILS_CPP_FILES) \
    $(UI_CPP_FILES) \
	$(IMGUI_CPP_FILES)

# Vendor C sources
VENDOR_C_FILES = \
    ../../vendor/cJSON.c \

# All sources
ALL_SOURCES = $(CPP_SOURCES) $(VENDOR_C_FILES)

# --- Object Files ---
OBJECTS_CPP = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(filter %.cpp,$(ALL_SOURCES)))
OBJECTS_C   = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$(ALL_SOURCES)))
OBJECTS     = $(OBJECTS_CPP) $(OBJECTS_C)

# --- Include Directories ---
INCLUDES = \
    -I../../vendor \
    -I../../vendor/sol \
    -I../../vendor/lua \
	-I../../vendor/imgui \
	-I../../vendor/imgui/backends/ \
	-I../../vendor/sqlite/

# --- Library Directories ---
LIB_DIRS = -L../../lib -L../../../boost.org/boost_1_88_0/build/lib

# --- Linker Flags and Libraries ---
STATIC_RUNTIME_FLAGS = -static-libgcc -static-libstdc++

# Libraries to link
# Verify these library names for your Boost installation (e.g., -lboost_system-mt or -lboost_system)
BOOST_LIBS = -lboost_system-mgw15-mt-s-x64-1_88 -lboost_thread-mgw15-mt-s-x64-1_88 -lboost_serialization-mgw15-mt-s-x64-1_88 -lssl -lcrypto -lcrypt32
PTHREAD_LIB = -Wl,--whole-archive -lwinpthread -Wl,--no-whole-archive # For static linking of pthreads if available

SDL_LIBS = -lsdl3 -lsdl3_image -lsdl3_ttf -lsdl3_mixer
WINDOWS_NET_LIBS = -lws2_32 -lwsock32
LUA_LIB = -llua # Or -llua5.4, etc.

# Order for linking
LINK_LIBS = \
    -Wl,-Bstatic \
    $(BOOST_LIBS) \
    $(PTHREAD_LIB) \
	-lsqlite3 \
	-Wl,-Bdynamic \
	$(LUA_LIB) \
    $(SDL_LIBS) \
    $(WINDOWS_NET_LIBS)

# Combined linker flags
LDFLAGS = $(LIB_DIRS) $(LINK_LIBS) -Wa,-mbig-obj

# --- DLL Filename Variables (for reference and ensuring correct files are in bin/) ---
# These variables define the expected filenames of the runtime DLLs
# that correspond to the dynamically linked libraries.
# The user is responsible for ensuring these DLL files (or their equivalents)
# are present in the OUTPUT_DIR (bin/) for the 'game' target to copy them,
# and for the application to run.
# !! THESE ARE EXAMPLES, VERIFY AND ADJUST FOR YOUR SYSTEM AND VERSIONS !!
BOOST_SYSTEM_DLL_NAME = libboost_system-mgwXY-mt-x64-1_YZ.dll # e.g., libboost_system-mgw12-mt-x64-1_80.dll
BOOST_THREAD_DLL_NAME = libboost_thread-mgwXY-mt-x64-1_YZ.dll # e.g., libboost_thread-mgw12-mt-x64-1_80.dll
LUA_DLL_NAME = liblua54.dll # Or liblua.dll, liblua5.1.dll etc.
# Common SDL DLLs: SDL3.dll, SDL3_image.dll, SDL3_ttf.dll
# Common MinGW/pthreads DLL: libwinpthread-1.dll
# If not statically linking libgcc/libstdc++, you might also need:
# libgcc_s_seh-1.dll, libstdc++-6.dll (names depend on GCC version)

# --- Primary Targets ---
.PHONY: all clean engine game test

all: $(TARGET)

# --- Rule to build the executable ---
$(TARGET): $(OBJECTS)
	@mkdir -p $(OUTPUT_DIR)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS) $(STATIC_RUNTIME_FLAGS)
	@echo "$(TARGET) built successfully into $(OUTPUT_DIR)/."

# --- Pattern rules for compiling source files to object files ---
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@echo "Compiling $< -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo "Compiling $< -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ # Using CXX for .c files as in original

# --- Utility Targets ---
engine: $(TARGET)

clean:
	-@rm -f $(TARGET)
	-@rm -rf $(OBJ_DIR)
	-@rm -rf $(BUILD_DIR) # Clean the 'game' build directory
	@echo "Cleaned project files and build artifacts."