TARGET := code
BUILD_DIR := build

SOURCES := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.hpp)
OBJECTS := $(SOURCES:src/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	g++ $(OBJECTS) -o $(TARGET)

$(BUILD_DIR)/%.o: src/%.cpp $(HEADERS) Makefile | $(BUILD_DIR)
	g++ -std=c++17 -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
