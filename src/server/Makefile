# Compiler
# Use GCC for C files for better practice, even if g++ can handle them.
CXX = g++
CC = gcc
WIN32_CXX = x86_64-w64-mingw32-g++
WIN32_CC = x86_64-w64-mingw32-gcc

# Compile flags
# Added -MMD -MP for automatic dependency generation
CXXFLAGS = -Wextra -std=c++17 -g -MMD -MP
CFLAGS = -Wall -std=c11 -g -MMD -MP # Specific C flags
WIN32_CXXFLAGS = -Wa,-mbig-obj -Wextra -std=c++17 -g -MMD -MP
WIN32_CFLAGS = -Wall -std=c11 -g -MMD -MP # Specific C flags for Windows

# --- Directories ---
OUTPUT_DIR_LINUX = bin/linux
OUTPUT_DIR_WIN32 = bin/win32
OBJ_DIR_LINUX = obj/linux
OBJ_DIR_WIN32 = obj/win32
DEPS_DIR_LINUX = deps/linux
DEPS_DIR_WIN32 = deps/win32

SERVER_TARGET_NAME_LINUX = server
SERVER_TARGET_LINUX = $(OUTPUT_DIR_LINUX)/$(SERVER_TARGET_NAME_LINUX)

SERVER_TARGET_NAME_WIN32 = server.exe
SERVER_TARGET_WIN32 = $(OUTPUT_DIR_WIN32)/$(SERVER_TARGET_NAME_WIN32)

# --- Source Files ---
# Core engine C++ files
SERVER_CPP_SOURCES = main.cpp Server.cpp

# Vendor C sources (explicitly for C compiler)
SERVER_C_SOURCES = \
    ../../vendor/cJSON.c \

# Combine all source files
ALL_SERVER_SOURCES_CPP = $(SERVER_CPP_SOURCES)
ALL_SERVER_SOURCES_C = $(SERVER_C_SOURCES)

# Object files for Linux
SERVER_OBJECTS_LINUX_CPP = $(patsubst %.cpp,$(OBJ_DIR_LINUX)/%.o,$(ALL_SERVER_SOURCES_CPP))
SERVER_OBJECTS_LINUX_C   = $(patsubst %.c,$(OBJ_DIR_LINUX)/%.o,$(ALL_SERVER_SOURCES_C))
SERVER_OBJECTS_LINUX     = $(SERVER_OBJECTS_LINUX_CPP) $(SERVER_OBJECTS_LINUX_C)

# Object files for Win32
SERVER_OBJECTS_WIN32_CPP = $(patsubst %.cpp,$(OBJ_DIR_WIN32)/%.o,$(ALL_SERVER_SOURCES_CPP))
SERVER_OBJECTS_WIN32_C   = $(patsubst %.c,$(OBJ_DIR_WIN32)/%.o,$(ALL_SERVER_SOURCES_C))
SERVER_OBJECTS_WIN32     = $(SERVER_OBJECTS_WIN32_CPP) $(SERVER_OBJECTS_WIN32_C)

# Dependency files for Linux
SERVER_DEPS_LINUX_CPP = $(patsubst %.cpp,$(DEPS_DIR_LINUX)/%.d,$(ALL_SERVER_SOURCES_CPP))
SERVER_DEPS_LINUX_C   = $(patsubst %.c,$(DEPS_DIR_LINUX)/%.d,$(ALL_SERVER_SOURCES_C))
SERVER_DEPS_LINUX     = $(SERVER_DEPS_LINUX_CPP) $(SERVER_DEPS_LINUX_C)

# Dependency files for Win32
SERVER_DEPS_WIN32_CPP = $(patsubst %.cpp,$(DEPS_DIR_WIN32)/%.d,$(ALL_SERVER_SOURCES_CPP))
SERVER_DEPS_WIN32_C   = $(patsubst %.c,$(DEPS_DIR_WIN32)/%.d,$(ALL_SERVER_SOURCES_C))
SERVER_DEPS_WIN32     = $(SERVER_DEPS_WIN32_CPP) $(SERVER_DEPS_WIN32_C)

# --- Include Directories ---
INCLUDES = \
    -I../../vendor \
    -I../../vendor/sol \
    -I../../vendor/lua \
	-I../../vendor/imgui \
	-I../../vendor/imgui/backends/ \
	-I../../vendor/sqlite/

# --- Library Directories ---
LIB_DIRS_LINUX = -L../../lib -L../../../boost.org/boost_1_88_0/build/lib
LIB_DIRS_WIN32 = -L../../lib -L../../../boost.org/boost_1_88_0/build/lib

# --- Linker Flags and Libraries ---
STATIC_RUNTIME_FLAGS = -static-libgcc -static-libstdc++

# Libraries to link
BOOST_LIBS = -lboost_system -lboost_thread -lboost_serialization
WIN32_BOOST_LIBS = -lboost_system-mgw15-mt-s-x64-1_88 -lboost_thread-mgw15-mt-s-x64-1_88 -lboost_serialization-mgw15-mt-s-x64-1_88

# SSL/Crypto libraries
SSL_CRYPTO_LIBS_LINUX = -lssl -lcrypto
SSL_CRYPTO_LIBS_WIN32 = -lssl -lcrypto -lcrypt32

# PThread library
PTHREAD_LIB_LINUX = -pthread
PTHREAD_LIB_WIN32 = -Wl,--whole-archive -lwinpthread -Wl,--no-whole-archive

# Networking libraries for Windows
WIN32_NET_LIBS = -lws2_32 -lwsock32

# Lua library
LUA_LIB = -llua5.4 # Or -llua

SERVER_LINK_LIBS_LINUX = \
	-Wl,-Bstatic \
	$(BOOST_LIBS) \
	$(PTHREAD_LIB_LINUX) \
	$(SSL_CRYPTO_LIBS_LINUX) \
	-lsqlite3 \
	-Wl,-Bdynamic \
	-llua5.4

SERVER_LINK_LIBS_WIN32 = \
	-Wl,-Bstatic \
	$(WIN32_BOOST_LIBS) \
	$(PTHREAD_LIB_WIN32) \
	$(SSL_CRYPTO_LIBS_WIN32) \
	-lsqlite3 \
	-Wl,-Bdynamic \
	-llua \
	$(WIN32_NET_LIBS)

SERVER_LDFLAGS_LINUX = $(LIB_DIRS_LINUX) $(SERVER_LINK_LIBS_LINUX)
SERVER_LDFLAGS_WIN32 = $(LIB_DIRS_WIN32) $(SERVER_LINK_LIBS_WIN32) -Wa,-mbig-obj

# --- DLL Handling ---
# List of DLLs to copy for Windows builds
WIN32_DLLS = \
    ../../lib/boost_system-mgw15-mt-s-x64-1_88.dll \
    ../../lib/boost_thread-mgw15-mt-s-x64-1_88.dll \
    ../../lib/lua54.dll \
    # Add other DLLs your project might need

# --- Primary Targets ---
.PHONY: all clean linux win32 run-linux run-win32

all: linux win32

linux: $(SERVER_TARGET_LINUX) $(SERVER_DEPS_LINUX)
win32: $(SERVER_TARGET_WIN32) $(SERVER_DEPS_WIN32)

# --- Rule to build the executables ---

$(SERVER_TARGET_LINUX): $(SERVER_OBJECTS_LINUX)
	@mkdir -p $(@D)
	@echo "Linking $(SERVER_TARGET_LINUX)..."
	$(CXX) $(SERVER_OBJECTS_LINUX) -o $(SERVER_TARGET_LINUX) $(SERVER_LDFLAGS_LINUX) $(STATIC_RUNTIME_FLAGS)
	@echo "$(SERVER_TARGET_LINUX) built successfully into $(OUTPUT_DIR_LINUX)/."

$(SERVER_TARGET_WIN32): $(SERVER_OBJECTS_WIN32)
	@mkdir -p $(@D)
	@echo "Linking $(SERVER_TARGET_WIN32)..."
	$(WIN32_CXX) $(SERVER_OBJECTS_WIN32) -o $(SERVER_TARGET_WIN32) $(SERVER_LDFLAGS_WIN32) $(STATIC_RUNTIME_FLAGS)
	@echo "$(SERVER_TARGET_WIN32) built successfully into $(OUTPUT_DIR_WIN32)/."
	@echo "Copying required DLLs to $(OUTPUT_DIR_WIN32)..."
	@cp $(WIN32_DLLS) $(OUTPUT_DIR_WIN32)/ 2>/dev/null || true # Suppress error if DLL not found, as it might be system-wide

# --- Pattern rules for compiling source files to object files ---

# Linux C++ compilation
$(OBJ_DIR_LINUX)/%.o: %.cpp
	@mkdir -p $(@D) $(DEPS_DIR_LINUX)
	@echo "Compiling $< -> $(@F) for Linux"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ -MF $(DEPS_DIR_LINUX)/$*.d

# Linux C compilation
$(OBJ_DIR_LINUX)/%.o: %.c
	@mkdir -p $(@D) $(DEPS_DIR_LINUX)
	@echo "Compiling $< -> $(@F) for Linux (C)"
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -MF $(DEPS_DIR_LINUX)/$*.d

# Win32 C++ compilation
$(OBJ_DIR_WIN32)/%.o: %.cpp
	@mkdir -p $(@D) $(DEPS_DIR_WIN32)
	@echo "Compiling $< -> $(@F) for Win32"
	$(WIN32_CXX) $(WIN32_CXXFLAGS) $(INCLUDES) -c $< -o $@ -MF $(DEPS_DIR_WIN32)/$*.d

# Win32 C compilation
$(OBJ_DIR_WIN32)/%.o: %.c
	@mkdir -p $(@D) $(DEPS_DIR_WIN32)
	@echo "Compiling $< -> $(@F) for Win32 (C)"
	$(WIN32_CC) $(WIN32_CFLAGS) $(INCLUDES) -c $< -o $@ -MF $(DEPS_DIR_WIN32)/$*.d

# --- Dependency Inclusion ---
# Include the .d files generated by the compilers.
# -include will not fail if the file does not exist (first build)
-include $(SERVER_DEPS_LINUX)
-include $(SERVER_DEPS_WIN32)

# --- Clean Target ---
clean:
	-@rm -rf $(OUTPUT_DIR_LINUX) $(OUTPUT_DIR_WIN32)
	-@rm -rf $(OBJ_DIR_LINUX) $(OBJ_DIR_WIN32)
	-@rm -rf $(DEPS_DIR_LINUX) $(DEPS_DIR_WIN32)
	@echo "Cleaned project files and build artifacts."

# --- Run Targets (Convenience) ---
run-linux: $(SERVER_TARGET_LINUX)
	@echo "Running Linux server..."
	./$(SERVER_TARGET_LINUX)

run-win32: $(SERVER_TARGET_WIN32)
	@echo "Running Windows server..."
	$(SERVER_TARGET_WIN32) # On Windows, you can directly execute .exe