CC = g++
CXX = g++

CFLAGS = -Wall
CXXFLAGS = -Wall

ARCH = $(shell if test `uname -m` = "x86_64"; then echo "x64"; else echo "x86"; fi)
BUILD_DIR = build/$(BUILD_ARCH)
SOURCE_DIR = src

SOURCES = $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS = $(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

TARGET = SDL2Boilerplate

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET)
