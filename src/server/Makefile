# Compiler
CXX = g++
WIN32_CXX = x86_64-w64-mingw32-g++

# For C files, we'll use CXX as well, assuming g++ can handle .c files
# or that CFLAGS compatible with C++ are used. If you need a specific C compiler:
# CC_C = x86_64-w64-mingw32-gcc

# Compile flags
CXXFLAGS = -Wextra -std=c++17 -g # Added -g for debug symbols
# CFLAGS_C = -Wall -std=c11 -g # Example if using a separate C compiler or flags
WIN32_CXXFLAGS = -Wa,-mbig-obj -Wextra -std=c++17 -g # Added -g for debug symbols

# --- Directories ---
OUTPUT_DIR = bin
OBJ_DIR = obj
BUILD_DIR = build

SERVER_TARGET_NAME = server.exe
SERVER_TARGET = $(OUTPUT_DIR)/$(SERVER_TARGET_NAME)

# --- Source Files ---
# Core engine C++ files
SERVER_CPP_FILES = main.cpp Server.cpp

# Vendor C sources
VENDOR_C_FILES = \
    ../../vendor/cJSON.c \


SERVER_OBJECTS_CPP = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(filter %.cpp,$(SERVER_CPP_FILES)))
SERVER_OBJECTS_C   = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$(SERVER_CPP_FILES)))
SERVER_OBJECTS     = $(SERVER_OBJECTS_CPP) $(SERVER_OBJECTS_C)

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
    # Add -L/path/to/your/boost_1_XX_X/stage/lib/ if Boost libs are not in default MinGW search paths

# --- Linker Flags and Libraries ---
STATIC_RUNTIME_FLAGS = -static-libgcc -static-libstdc++

# Libraries to link
# Verify these library names for your Boost installation (e.g., -lboost_system-mt or -lboost_system)
#BOOST_LIBS = -lboost_system-mgw15-mt-s-x64-1_88 -lboost_thread-mgw15-mt-s-x64-1_88 -lboost_serialization-mgw15-mt-s-x64-1_88 -lssl -lcrypto -lcrypt32
BOOST_LIBS = -lboost_system -lboost_thread -lboost_serialization -lssl -lcrypto -lcrypt
WIN32_BOOST_LIBS = -lboost_system-mgw15-mt-s-x64-1_88 -lboost_thread-mgw15-mt-s-x64-1_88 -lboost_serialization-mgw15-mt-s-x64-1_88 -lssl -lcrypto -lcrypt32
#PTHREAD_LIB = -Wl,--whole-archive -lwinpthread -Wl,--no-whole-archive # For static linking of pthreads if available
PTHREAD_LIB=
WIN32_PTHREAD_LIB = -Wl,--whole-archive -lwinpthread -Wl,--no-whole-archive

WIN32_NET_LIBS = -lws2_32 -lwsock32

WIN32_LUA_LIB = -llua

LUA_LIB = -llua5.4 # Or -llua5.4, etc.
L_LUA_LIB = -llua5.4 # Or -llua5.4, etc.
SERVER_LINK_LIBS = \
	-Wl,-Bstatic \
	$(BOOST_LIBS) \
	$(PTHREAD_LIB) \
	-lsqlite3 \
	-Wl,-Bdynamic \
	$(LUA_LIB) \
	$(WINDOWS_NET_LIBS)
WIN32_SERVER_LINK_LIBS = \
	-Wl,-Bstatic \
	$(WIN32_BOOST_LIBS) \
	$(WIN32_PTHREAD_LIB) \
	-lsqlite3 \
	-Wl,-Bdynamic \
	$(WIN32_LUA_LIB) \
	$(WIN32_NET_LIBS)

SERVER_LDFLAGS = $(LIB_DIRS) $(SERVER_LINK_LIBS)
WIN32_SERVER_LDFLAGS = $(LIB_DIRS) $(WIN32_SERVER_LINK_LIBS) -Wa,-mbig-obj

BOOST_SYSTEM_DLL_NAME = libboost_system-mgwXY-mt-x64-1_YZ.dll # e.g., libboost_system-mgw12-mt-x64-1_80.dll
BOOST_THREAD_DLL_NAME = libboost_thread-mgwXY-mt-x64-1_YZ.dll # e.g., libboost_thread-mgw12-mt-x64-1_80.dll
LUA_DLL_NAME = liblua54.dll

# --- Primary Targets ---
.PHONY: all clean engine game test

linux: $(SERVER_TARGET)-linux
win32: $(SERVER_TARGET)-win32

# --- Rule to build the executable ---

$(SERVER_TARGET)-linux: linux-$(SERVER_OBJECTS)
	@mkdir -p $(OUTPUT_DIR)
	@echo "Linking $(SERVER_TARGET)..."
	$(CXX) $(SERVER_OBJECTS) -o $(SERVER_TARGET) $(SERVER_LDFLAGS) $(STATIC_RUNTIME_FLAGS)
	@echo "$(SERVER_TARGET) built successfully into $(OUTPUT_DIR)/."

$(SERVER_TARGET)-win32: win32-$(SERVER_OBJECTS)
	@mkdir -p $(OUTPUT_DIR)
	@echo "Linking $(SERVER_TARGET)..."
	$(WIN32_CXX) $(SERVER_OBJECTS) -o $(SERVER_TARGET) $(WIN32_SERVER_LDFLAGS) $(STATIC_RUNTIME_FLAGS)
	@echo "$(SERVER_TARGET) built successfully into $(OUTPUT_DIR)/."

# --- Pattern rules for compiling source files to object files ---
linux-$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@echo "Compiling $< -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
win32-$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@echo "Compiling $< -> $@"
	$(WIN32_CXX) $(WIN32_CXXFLAGS) $(INCLUDES) -c $< -o $@

linux-$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo "Compiling $< -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ # Using CXX for .c files as in original
win32-$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo "Compiling $< -> $@"
	$(WIN32_CXX) $(WIN32_CXXFLAGS) $(INCLUDES) -c $< -o $@ # Using CXX for .c files as in original
	
clean:
	-@rm -f $(SERVER_TARGET)
	-@rm -rf $(OBJ_DIR)
	-@rm -rf $(BUILD_DIR)
	@echo "Cleaned project files and build artifacts."
