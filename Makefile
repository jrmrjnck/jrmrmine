#### PROJECT SETTINGS ####
# The name of the executable to be created
BIN_NAME := jrmrmine
# Compiler used
CXX ?= g++
# Extension of source files used in the project
SRC_EXT = cpp
# Path to the source directory, relative to the makefile
SRC_PATH = .
# General compiler flags
COMPILE_FLAGS = -std=c++11 -Wall -g
# Additional release-specific flags
RCOMPILE_FLAGS = -D NDEBUG -O3 -funroll-loops
# Additional debug-specific flags
DCOMPILE_FLAGS = -D DEBUG
# Add additional include paths
INCLUDES = -I $(SRC_PATH)/
# General linker settings
LINK_FLAGS = -lcurl -ljsoncpp -lboost_program_options
# Additional release-specific linker settings
RLINK_FLAGS = 
# Additional debug-specific linker settings
DLINK_FLAGS = 
#### END PROJECT SETTINGS ####

# Generally should not need to edit below this line

# Shell used in this makefile
# bash is used for 'echo -en'
SHELL = /bin/bash
# Clear built-in rules
.SUFFIXES:

# Verbose option, to output compile and link commands
export V = true
export CMD_PREFIX = @
ifeq ($(V),true)
	CMD_PREFIX = 
endif

# Combine compiler and linker flags
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RCOMPILE_FLAGS)
release: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RLINK_FLAGS)
debug: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS)
debug: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DLINK_FLAGS)

# Build and output paths
release: export BUILD_TYPE := release
debug:   export BUILD_TYPE := debug
release debug: export BUILD_PATH := build/$(BUILD_TYPE)
release debug: export BIN_PATH := bin/$(BUILD_TYPE)
release debug: export TARGET := $(BIN_PATH)/$(BIN_NAME)

# Find all source files in the source directory
SOURCES = $(shell find $(SRC_PATH)/ -name '*.$(SRC_EXT)')
# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)
# Set the dependency files that will be used to add header dependencies
DEPS = $(OBJECTS:.o=.d)

.PHONY: debug release
debug release:
	@$(MAKE) build --no-print-directory

all: debug release

build: dirs $(TARGET)

.PHONY: dirs
dirs: $(BUILD_PATH) $(BIN_PATH)

$(BUILD_PATH) $(BIN_PATH):
	@mkdir -p $@

# Removes all build files
.PHONY: clean
clean:
	$(RM) $(BIN_NAME)
	$(RM) -r build
	$(RM) -r bin

# Link the executable
$(TARGET): $(OBJECTS)
	$(CMD_PREFIX)$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	ln -sf $@ $(BIN_NAME)

# Add dependency files, if they exist
-include $(DEPS)

# Source file rules
# After the first compilation they will be joined with the rules from the
# dependency files to provide header dependencies
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	$(CMD_PREFIX)$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@
